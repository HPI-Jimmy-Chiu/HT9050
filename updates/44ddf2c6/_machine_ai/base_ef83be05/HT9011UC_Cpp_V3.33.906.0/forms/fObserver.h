// =============================================================================
//  forms/fObserver.h  --  non-VCL facade for golden's TfObserver (cObserver.h)
//
//  AI(W906-FW3-Observer-W1) 20260818: new file, FW-3 cObserver Wave 1.
//  Golden ref: HT9011UC_Code_V3.33.906.0_20260618/cObserver.h (567 lines) +
//  cObserver.cpp (5,425 lines, cp950/Big5 -- decoded with `python3 -c
//  "open(...).read().decode('cp950')"`, 0 U+FFFD, this wave).
//
//  ROLE
//  ----
//  TfObserver is golden's "OEE Observer" dialog: Counter/TestCate/Scanner/
//  MDBQuery/Yield/TestInfo/EventLogTxt/Precautions/LotInfo tabs. This file
//  declares ONLY the facade surface Wave 1's translated methods dereference
//  (cObserver.cpp, this same wave) -- not TfObserver's full ~260-member
//  golden surface. Later cObserver waves grow this header additively, same
//  convention as forms/fMain.h.
//
//  THIS IS A SEPARATE, NEW CLASS -- NOT THE EXISTING SHIM
//  --------------------------------------------------------------------------
//  atester_shims.h/.cpp ALREADY declare a `class TfObserverShim` and a global
//  `TfObserverShim *fObserver` (golden cObserver.h's own `extern PACKAGE
//  TfObserver *fObserver;`, landed by an earlier wave for the ~7 members other
//  translated files needed). This header does NOT touch either -- no `fObserver`
//  global is defined here (not even `extern`); the shim-vs-facade swap is an
//  INTEGRATION decision the main loop makes later (same class of decision as
//  the two-TMyKitSuck headers -- see docs/KNOWLEDGE.md). Until that swap,
//  `class TfObserver` below has no live instance anywhere in the tree; it is
//  exercised only by tests/test_observer_core.cpp (this wave), which
//  constructs its own local instance.
//
//  THE FACADE CONTRACT (same rules as forms/fMain.h, summarised)
//  --------------------------------------------------------------------------
//  1. Methods that are GENUINE TRANSLATIONS this wave (real, faithful bodies
//     in cObserver.cpp) are `virtual` per the tree-wide convention (allows a
//     future MFC binder to override), EXCEPT CalculateStopTime/
//     UnCalculateStopTime -- see the DEVIATION note on those two below.
//  2. Data members are concrete storage, never accessors (same rationale as
//     fMain.h: BCB6 `__property` has no C++ equivalent, and call sites read
//     `fObserver->widget->Prop` directly).
//  3. Widget stand-ins: stock VCL controls (TComboBox/TListBox/TRadioGroup/
//     TPanel) reuse vclcompat/Controls.h directly, same as every other form.
//     TStringGrid/TTMyTray/TChart/TDateTimePicker need EXTRA surface golden
//     had (ColWidths, FixedRows, Rows[]->Clear(), Series[]/Title, Date/
//     DateTime) that vclcompat's existing shims don't carry (by design -- see
//     vclcompat/StringGrid.h and vclcompat/TrayCore.h's own SCOPE notes), so
//     this wave defines four small facade-only wrapper types below
//     (TfObserverGrid / TfObserverTray / TfObserverChart(+Series) /
//     TfObserverDateTimePicker) rather than editing those shared vclcompat
//     headers (out of this wave's write boundary; DESIGN NOTES below explain
//     each shape and why it composes the existing shim instead of forking it).
//
//  WAVE SCOPE (every golden method this header declares, golden line span,
//  ACTIVE / GATED / DEVIATION)
//  --------------------------------------------------------------------------
//    TfObserver()                        golden :137-346 (ctor)      ACTIVE
//    SetSiteYieldDiagram                 golden :771-837             ACTIVE
//    UpdateYieldChart                    golden :838-864             ACTIVE
//    SpeedButton1Click                   golden :865-872             ACTIVE
//    UpdateBin                           golden :873-904             ACTIVE
//    CalculateStopTime                   golden :1686-1707           ACTIVE, DEVIATION (public+static, was private+instance -- see below)
//    WriteContactKind                    golden :1301-1648           ACTIVE
//    UnCalculateStopTime                 golden :2237-2253           ACTIVE, DEVIATION (public+static, was private+instance -- see below)
//    BtnQueryClick                       golden :2421-2776           ACTIVE, 2 GATED sub-blocks (see GATE REGISTER 1, 2)
//    btReportClick                       golden :3186-3206           ACTIVE
//    rgContactCountKindsClick            golden :3219-3223           ACTIVE
//    rgContactCountHistoryClick          golden :3225-3229           ACTIVE
//    rgContactCountKindsFormClick        golden :3231-3235           ACTIVE
//    rgContactCountHistoryFormClick      golden :3237-3241            ACTIVE
//    lstEventLogClick                    golden :3757-3762           ACTIVE
//    cbbMonthChange                      golden :3763-3799           ACTIVE
//    GetEventLogText                     golden :3801-3971           ACTIVE
//    btnQueryEventLogTxtClick            golden :4423-4426           ACTIVE
//    DoProduction_Summary_Report(4 args) golden cObserver.h:552      GATED (body outside Wave-1 byte range; called from
//                                                                     BtnQueryClick's Production_Summary_Report branch --
//                                                                     documented no-op stub, see GATE REGISTER 3)
//    CountMTBF()                         golden cObserver.h:517      GATED (body outside Wave-1 byte range; called from
//                                                                     BtnQueryClick's AMKOR/Microchip MTBF tail --
//                                                                     documented no-op stub, see GATE REGISTER 3)
//
//  AI(W906-FW3-Observer-W2) 20260825 -- SCOPE OF THE TABLE ABOVE, stated so it
//  is not mistaken for a full inventory: it lists WAVE 1 ONLY. Wave 2's eleven
//  methods (GetMachineData .. RecordIndexCycle) were declared without being
//  added here, and Wave 3's fifty-eight are not here either. Rather than let a
//  hand-kept table drift a third time, the authoritative per-method record now
//  lives ON THE DECLARATIONS themselves: every Wave-2 and Wave-3 declaration
//  below carries its golden line span in a trailing comment, and Wave 3's spans
//  were MEASURED out of golden rather than transcribed (19 of the 58 the
//  translation chunks cited were wrong). ACTIVE/GATED status likewise lives at
//  each method's own banner in cObserver.cpp, which is the only place that
//  cannot drift from the code.
//
//  WAVE 3 SUMMARY (the numbers, so this header still answers "how much"):
//    58 methods merged 20260825 -- 13 display/lifecycle, 28 Precaution /
//    MajorMaintenance record, 17 event-log & misc handlers.
//    95 new members here = 88 widget pointers (20 tab/page + 68 that live
//    translated statements dereference) + 7 scalars (bChangeRow[2],
//    bIsLoaded[13] and the five Precaution-record flags). bTabVisible[20] is
//    NOT among them: golden declares it at file scope (golden cObserver.cpp:57)
//    and it is translated there, in cObserver.cpp.
//    Gates: chunk A came with 40 blocks and ends with 24 -- FW3A-2, FW3A-5,
//    FW3A-6 and FW3A-9 were all retired at integration after their premises
//    were re-measured and failed (details in that chunk's own register).
//    C-log-7/C-log-8 retired via a 4-field LeftAxis model; C-log-1 narrowed
//    from a whole branch down to one ShowModal call.
//
//  DEVIATION -- CalculateStopTime/UnCalculateStopTime made public+static
//  --------------------------------------------------------------------------
//  Golden declares both `private` and non-static (cObserver.h:486-487). Read
//  in full this wave: NEITHER touches `this` anywhere in its body (pure
//  functions of their one parameter + locals -- verified by reading golden
//  cObserver.cpp:1686-1707/:2237-2253 end to end). Keeping them
//  private+instance would force tests/test_observer_core.cpp to construct a
//  full TfObserver (which, for OTHER reasons -- see GetObserAuth/
//  CheckAndReadIniDataGeneral below -- needs real, if redirected, file I/O)
//  just to exercise two pure string/int conversions. `static` is the minimal,
//  documented, behaviour-preserving relaxation (same class of deliberate
//  access-level liberty this tree already takes with `virtual`, which golden
//  also never wrote). Call sites inside this class (none in Wave 1 -- both are
//  leaf functions with no internal callers in golden's own line ranges) are
//  unaffected either way.
//
//  GATE REGISTER
//  --------------------------------------------------------------------------
//  (1) BtnQueryClick, golden :2668-2684 (the Alarm_Code_List per-row level/
//      silent/red-background loop). Needs `fMain->AlarmUnitMap` (grepped
//      forms/fMain.h in full this wave -- 0 hits, no such member) AND a
//      `fSecurity` facade (grepped forms/*.h -- no fSecurity.h exists in the
//      tree at all, only fAGV/fAOI/fCleaning/fFixAICCD/fHome/fLotInfo/fMain/
//      fNote/fOCR/fOffSet/fProductionInfo/fRotate/fSCKART/fSetup/
//      fShowMessage/fShuttleMove/fSortCT/fTrayForm). Two independent missing
//      dependencies, same "documented GAP, not silently dropped" idiom as
//      fMain.h's ATC_InterfaceForm/fBinSel gates. The grid columns this loop
//      would fill (Cells[4..6]) are simply left at whatever MyDBVProcess
//      populated (blank), which is the faithful "no security-level lookup
//      happened" default.
//  (2) BtnQueryClick, golden :2723/:2726/:2728/:2730/:2732/:2734 (6 calls:
//      1x MyDBVUnitEventCount + 5x MyDBVAxleEventCount). cMyDB.h:75 keeps
//      `class TChart;` a permanently OPAQUE forward decl ("TeeChart VCL
//      component -- NOT ported, W7-UI scope") and cMyDB.cpp itself ALREADY
//      gates both functions' BODIES for exactly that reason (cMyDB.h:112-113
//      "BODY GATED (no TChart port, see file head)"). This facade's Chart2 is
//      TfObserverChart* (this wave's own data-only stand-in, DESIGN NOTES
//      below) -- a deliberately DIFFERENT type from golden's opaque `TChart*`,
//      so passing it to either function cannot even bind at the type level,
//      not just "the body would no-op". Every OTHER Chart2 touch in this same
//      function (Visible, Series[0]->Clear(), Title->Text->Clear()/Add()) is
//      real and ACTIVE against TfObserverChart -- only the two golden library
//      calls that need the opaque TChart* are gated.
//  (3) DoProduction_Summary_Report / CountMTBF -- both are golden TfObserver
//      MEMBER functions (cObserver.h:552/:517) whose BODIES sit outside every
//      one of Wave 1's assigned golden byte ranges (grepped: neither name
//      appears inside cObserver.cpp:137-346/771-904/1301-1648/1686-1707/
//      2237-2253/2421-2776/3186-3242/3757-3971/4423-4426). BtnQueryClick
//      calls both (golden :2688-2691 and :2770), so this header declares them
//      with a documented no-op stub body (cObserver.cpp, this wave) -- same
//      "GAP stub, not a silent drop" idiom as forms/fMain.h's
//      SetMainRunStartMode.
//
//  DESIGN NOTES -- the four facade-only widget wrapper shapes
//  --------------------------------------------------------------------------
//  TfObserverGrid : public vclcompat::TStringGrid
//    golden TStringGrid ALSO exposes ->ColWidths[i]= (used ~30 times across
//    Wave 1's ctor/GetEventLogText/BtnQueryClick/btReportClick),
//    ->DefaultColWidth=, ->FixedRows=, ->Visible (strngrdMDBQuery only),
//    ->Rows[i]->Clear() (a whole-row wipe with no Cells-level equivalent)
//    and ->Repaint()/->Refresh() (StringGrid2/3, GDI-rendering-only, no-op
//    this wave -- see the "GDI methods not this wave" project rule).
//    AI(W906-VclGrid-1) 20260820: Visible/DefaultColWidth/FixedRows/
//    ColWidths[]/ClearRow(i) MOVED to the shared vclcompat::TStringGrid base
//    (vclcompat/StringGrid.h) as part of the 5-subclass TStringGrid-extension
//    consolidation wave -- this facade's own copies were a byte-for-byte
//    duplicate of forms/fConfiguration.h's TfConfigurationGrid, so they
//    collapsed home instead of risking a third fork. TfObserverGrid PUBLICLY
//    INHERITS vclcompat::TStringGrid (so it still converts for free to every
//    `TStringGrid*` cMyDB.h function -- MyDBVProcess/MyDBVProcessFilter/
//    MyDBVEventFreq/MyDBQTimeData all take `vclcompat::TStringGrid*`) and now
//    keeps ONLY its two GDI no-ops (Repaint()/Refresh(), StringGrid2/3 --
//    out of scope for the base, which carries no rendering surface at all)
//    plus a pass-through hydration ctor. See vclcompat/StringGrid.h's own
//    updated SCOPE banner for the base's full current member set and the
//    verified-inert reasoning behind dropping the ctor's old eager
//    ColWidths pre-sizing.
//
//  TfObserverTray : public vclcompat::TObject, composing vclcompat::TrayCore
//    golden TTMyTray (HTray.h/.cpp) already has a real, tested, framework-free
//    core: vclcompat/TrayCore.h's `TrayCore` (W7-C1), which implements
//    SetColorMap/SetCellNumber/SetCellColorIndex/SetYItem with the SAME
//    documented golden bugs golden's real HTray.cpp has (e.g. SetYItem,
//    ConvertIndexCells -- see TrayCore.h's own per-method "GOLDEN BUG, see
//    .cpp" tags). Wave 1's mtRow[]/mtCategorySum/.../mtCategoryTotal members
//    call exactly SetColorMap/SetCellNumber/SetCellColorIndex (never
//    CaculateTrayParameter/CellRect/ConvertIndexCells directly -- those three
//    are GDI/hit-test geometry, out of scope this wave per the project's
//    DrawCell*/DrawCenterLine exclusion), so composing TrayCore gives Wave 1
//    real, already-independently-verified behaviour for free -- reusing
//    tested infrastructure beats re-deriving it. TrayCore itself has no
//    Visible/Tag (those are golden TCustomControl/TComponent properties, one
//    layer up, that TrayCore's own SCOPE note explicitly keeps out of a
//    "state/geometry core"), so this wrapper adds them as plain facade
//    fields. `->Width=`/`->YItem=` (VCL property WRITES, golden ctor/
//    SetSiteYieldDiagram) route to Core.Width (plain field write -- TrayCore's
//    own note: direct field writes do NOT recompute geometry, only the
//    NEW-not-golden SetExtents() does, and no Wave-1 method ever reads
//    geometry back) and Core.SetYItem() (the real mutator, preserving its
//    documented golden bug) respectively.
//
//  TfObserverChart / TfObserverChartSeries : public vclcompat::TObject
//    Golden ChartYield/TempChart/Chart2 are all `TChart*` (TeeChart, golden
//    cObserver.h). Project-wide, `TChart` is a DELIBERATELY OPAQUE forward
//    decl (cMyDB.h:75, "NOT ported, W7-UI scope") -- so this facade cannot use
//    that name for a widget that needs REAL behaviour (ChartYield/
//    UpdateYieldChart's Series[]->Clear()/->AddY()/->Active ARE genuinely
//    ACTIVE this wave, not gated). TfObserverChart is this wave's own,
//    differently-named, minimal data-only stand-in: a growable vector of
//    TfObserverChartSeries (Title/Active/Points, where Points models golden's
//    ->AddY(value,label,color) triples), a Title.Text (reuses TStringList,
//    matching golden Chart->Title->Text being a TStrings*), and Visible. Being
//    a DIFFERENT type from the opaque `::TChart` is precisely why the two
//    library calls in GATE REGISTER (2) cannot bind -- documented there, not
//    a defect here. ChartYield needs 32 PRE-EXISTING series (golden's .dfm
//    wires 32 named TLineSeries SeriesAa..SeriesDh onto it at design time;
//    UpdateYieldChart's `for(iRow<MAX_SOCKET_TOTAL) ChartYield->Series[iRow]`
//    assumes they already exist) and Chart2 needs 1 (golden's .dfm's single
//    BarSeries2) -- both PRE-POPULATED in TfObserver's own ctor as PORT-ONLY
//    bootstrapping (a headless facade has no .dfm to instantiate named
//    child components from), clearly separated in cObserver.cpp from the
//    golden-line-by-line ctor translation. TempChart starts with ZERO
//    (golden's OWN ctor code dynamically AddSeries()-populates it,
//    genuinely translated, not bootstrapped).
//
//  TfObserverDateTimePicker : public vclcompat::TObject
//    golden TDateTimePicker exposes ->Date and ->DateTime (both TDateTime).
//    No vclcompat TDateTimePicker stand-in exists yet anywhere in the tree;
//    plain data holder, same shape as every other facade widget.
// =============================================================================
#ifndef FORMS_FOBSERVER_H
#define FORMS_FOBSERVER_H

#include "vclcompat/vcl_compat.h"     // AnsiString, TStringList, TDateTime -- brought to global scope
#include "vclcompat/Controls.h"       // TComboBox/TListBox/TRadioGroup/TPanel/TRadioButton (stock widgets, reused as-is)
#include "vclcompat/StringGrid.h"     // vclcompat::TStringGrid -- TfObserverGrid's base
#include "vclcompat/FileListBox.h"    // AI(W906-FW3-Observer-W2) 20260825: vclcompat::TFileListBox -- FileListBoxPrecautionLog / FileListBoxMajorMaintenance (golden cObserver.h:283/:257)
#include "vclcompat/TrayCore.h"       // vclcompat::TrayCore -- TfObserverTray's composed core
// AI(W906-FW3-Observer-W2) 20260818: myTimer.h -- TQPF_Timer, golden TfObserver::
// tRecordInArmTimer (cObserver.h:549). Real, already-ported class (RecordInArmTime,
// this wave) -- QueryPerformanceCounter-backed, NOT a facade stand-in.
#include "myTimer.h"
// AI(W906-FW-ObsSwap) 20260818: <vector> for TfObserverMemoLotSummaryLines
// (moved here from atester_shims.h with the shim retirement, see below).
#include <vector>
// AI(W906-FW3-Observer-W2) 20260818: MachineType.h -- tcTotalCount (the
// dTempHistroy[tcTotalCount][60] member, UpdateTempChart). Not transitively
// pulled in by any header above; cObserver.cpp itself also includes this
// directly (Wave 1) but a HEADER member array needs it visible right here too.
#include "MachineType.h"
#include "vclcompat/ShiftState.h"   // AI(W906-FW-SIG-W15) 20260826

using vclcompat::TComboBox;
using vclcompat::TListBox;
using vclcompat::TRadioGroup;
using vclcompat::TRadioButton;
using vclcompat::TPanel;
using vclcompat::TStringGrid;

// golden Graphics.hpp TColor, at GLOBAL scope (matching cmydef.h:16 and every
// other TU-local shim in the tree -- see cObserver.cpp's own colour-shim
// banner). NOT vclcompat::TColor (TrayCore.h defines that name, but inside
// `namespace vclcompat`, so it would not resolve here unqualified).
// vclcompat/Controls.h deliberately keeps its own Color members plain `int`
// rather than pull in a TColor definition (see that header's own note); this
// facade's Chart series DOES need the golden property name for the
// ->AddY(value,label,color) triple, so it defines the one-line alias here.
#ifndef HT9045_W906_FOBSERVER_TCOLOR_SHIM
#define HT9045_W906_FOBSERVER_TCOLOR_SHIM
typedef int TColor;
#endif

// ===========================================================================
//  TfObserverGrid -- see DESIGN NOTES above.
// ===========================================================================
class TfObserverGrid : public vclcompat::TStringGrid
{
public:
    // golden .dfm design-time ColCount/RowCount, read from
    // tools/dfm2rc/ir_out/cObserver.dfm.ir.json this wave (see per-member call
    // sites in cObserver.cpp for the exact citation of which grid gets which
    // pair) -- NOT guessed: real values recorded at form-design time.
    //
    // AI(W906-VclGrid-1) 20260820: members already collapsed to the base
    // (this class's own SCOPE-comment above has the provenance) -- this
    // class only remains as a hydration ctor plus two GDI no-op overrides.
    explicit TfObserverGrid(int initialColCount = 5, int initialRowCount = 5)
        : vclcompat::TStringGrid(initialColCount, initialRowCount)
    {}

    // golden ->Repaint()/->Refresh() -- GDI repaint triggers (StringGrid2/3's
    // 4 rgContactCount*Click handlers). Out of scope this wave (DrawCell*/
    // DrawCenterLine family exclusion) -- no-op, matching that exclusion.
    void Repaint() {}
    void Refresh() {}
};

// ===========================================================================
//  TfObserverTray -- see DESIGN NOTES above.
// ===========================================================================
class TfObserverTray : public vclcompat::TObject
{
public:
    vclcompat::TrayCore Core;   // golden TTMyTray state/geometry core (vclcompat/TrayCore.h, HTray.h/.cpp)
    bool Visible;               // golden TCustomControl->Visible -- offline default false
    int  Tag;                   // golden TComponent->Tag

    TfObserverTray() : Visible(false), Tag(0) {}
    virtual ~TfObserverTray() {}
};

// ===========================================================================
//  TfObserverChart / TfObserverChartSeries -- see DESIGN NOTES above.
// ===========================================================================
class TfObserverChartSeries : public vclcompat::TObject
{
public:
    struct Point { double Value; AnsiString Text; TColor Color; };

    AnsiString         Title;    // golden TChartSeries->Title (TempChart's per-series set in ctor)
    bool               Active;   // golden TChartSeries->Active (UpdateYieldChart)
    std::vector<Point> Points;   // golden ->AddY(value,label,color) triples, in append order

    TfObserverChartSeries() : Active(false) {}
    virtual ~TfObserverChartSeries() {}

    void Clear() { Points.clear(); }
    void AddY(double value, const AnsiString &text, TColor color)
    {
        Point p; p.Value = value; p.Text = text; p.Color = color;
        Points.push_back(p);
    }
};

class TfObserverChart : public vclcompat::TObject
{
public:
    struct TitleHolder
    {
        TStringList *Text;   // golden Chart->Title->Text (a TStrings*) -- only ->Clear()/->Add() used
        TitleHolder() { Text = new TStringList(); }
        ~TitleHolder() { delete Text; }
    };

    // AI(W906-FW3-Observer-W2) 20260825: golden TeeChart TChart->LeftAxis, used
    // by edYieldMaxClick/edYieldMinClick (golden :3210/:3216) to rescale the
    // Yield chart from two operator-editable fields. Only Maximum/Minimum are
    // touched anywhere in golden's cObserver, so only those two are modelled.
    // Write-only in this port -- no renderer reads them back yet -- but that is
    // exactly the state a web view will need, and keeping the writes live is
    // what makes the two handlers faithful instead of half-translated.
    struct AxisHolder
    {
        double Maximum;
        double Minimum;
        AxisHolder() : Maximum(0.0), Minimum(0.0) {}
    };

    bool        Visible;
    TitleHolder Title;
    // Heap-allocated rather than a pointer to an inline member: golden spells it
    // `->LeftAxis->Maximum`, so a pointer is required, and a pointer INTO this
    // object would silently alias the original if the chart were ever copied.
    // Same ownership idiom as TitleHolder::Text just above.
    AxisHolder *LeftAxis;

    TfObserverChart() : Visible(false), LeftAxis(new AxisHolder()) {}
    virtual ~TfObserverChart() { delete LeftAxis; for (size_t i = 0; i < seriesList_.size(); ++i) delete seriesList_[i]; }

    // golden ->AddSeries(new TLineSeries(...)) -- this facade drops the VCL
    // owner-ctor-arg (no .dfm ownership tree offline); appends and returns
    // the new series so `Chart->Series[j]->Title=...;` right after (golden's
    // own idiom in the TempChart ctor loop) keeps working.
    TfObserverChartSeries *AddSeries(TfObserverChartSeries *s)
    {
        seriesList_.push_back(s);
        return s;
    }

    struct SeriesAccessor
    {
        TfObserverChart *owner;
        TfObserverChartSeries *operator[](int idx) const { return owner->seriesList_.at(static_cast<size_t>(idx)); }
    };
    SeriesAccessor Series{this};

    // golden ->Repaint()/->Refresh() -- GDI, no-op (see TfObserverGrid note).
    void Repaint() {}
    void Refresh() {}

private:
    std::vector<TfObserverChartSeries *> seriesList_;
};

// ===========================================================================
//  TfObserverDateTimePicker -- see DESIGN NOTES above.
// ===========================================================================
class TfObserverDateTimePicker : public vclcompat::TObject
{
public:
    TDateTime Date;       // golden TDateTimePicker->Date
    TDateTime DateTime;   // golden TDateTimePicker->DateTime
    // AI(W906-FW3-Observer-W2) 20260818: golden TDateTimePicker->Time (CountMTBF,
    // this wave, DateTimePicker2/4). Real VCL TDateTimePicker holds ONE TDateTime
    // value and Date/Time/DateTime are three views of it that differ only by
    // which part a given ->Kind (dtDate/dtTime) exposes; this headless facade has
    // no Kind and no single backing value, so (matching this class's own existing
    // Date/DateTime split, Wave 1) Time is a THIRD independent stored field, not
    // a computed alias. Nothing this wave's translated code WRITES any of the
    // three -- all three are populated from outside (the not-yet-built UI layer,
    // or a test) -- so three independent fields cannot desync a call path that
    // never cross-reads them.
    TDateTime Time;       // golden TDateTimePicker->Time

    TfObserverDateTimePicker() {}
    virtual ~TfObserverDateTimePicker() {}
};

// ===========================================================================
//  TfObserverPageControl -- golden TPageControl, plus the ->ActivePage pointer.
//
//  AI(W906-FW3-Observer-W2) 20260825: vclcompat::TPageControl models only
//  ->ActivePageIndex (Controls.h:478-483). golden's cObserver ALSO uses
//  ->ActivePage, the TTabSheet POINTER, in both directions:
//      write  golden cObserver.cpp:608  pgcObserv->ActivePage=tsHanderMajorMaintenance;
//      write  golden cObserver.cpp:671  pgcPrecautions->ActivePage=tsPrecautionsRecord;
//      READ   golden cObserver.cpp:2384 pgcObserv->ActivePage==tsOEE_ProductionInfor
//  In real VCL the two are views of one selection; this headless facade has no
//  tab strip to reconcile them against, so ActivePage is an independent stored
//  pointer -- exactly the shape forms/fLotInfo.h:815-820 already established
//  for TfLotInfoPageControl, and for the same reason (that class's own note).
//
//  Defaults NULL: offline, no tab is "definitively active" until something
//  assigns one, so the :2384 read falls through to its else -- the same
//  conservative-default posture as every Visible/Enabled default in this
//  facade. Note the two are NOT kept in sync with each other: nothing in the
//  translated code ever writes one and reads the other, which is what makes
//  two independent fields safe here rather than merely convenient.
// ===========================================================================
class TfObserverPageControl : public vclcompat::TPageControl
{
public:
    vclcompat::TTabSheet *ActivePage;
    TfObserverPageControl() : ActivePage(0) {}
    virtual ~TfObserverPageControl() {}
};

// ===========================================================================
//  W906Obs2_InstanceRegistrar -- PORT-ONLY, NOT a golden type.
//
//  AI(W906-FW3-Observer-W2) 20260818: golden's file-scope free functions
//  IniRecordMonitoringIndexCycleTime/RecordIndexAirOnTime1/RecordIndexAirOnTime2
//  (golden cObserver.h:563-565) are NOT TfObserver members -- they reach the
//  single live form through golden's own module-level `fObserver` global
//  (e.g. `fObserver->iIndexCycleTimeCount=0;`). This header deliberately does
//  NOT redeclare that global here (Wave 1's own "integration-pending" note,
//  above: the live `fObserver` is already `TfObserverShim*`, atester_shims.h --
//  a second, differently-typed `fObserver` would collide exactly like the
//  two-TMyKitSuck-headers trap). Those 3 free functions still need SOME way to
//  reach the one constructed `TfObserver`, and Wave 1's own ctor BODY
//  (cObserver.cpp) is existing, untouched content this wave may not edit
//  (append-only rule) -- so it cannot simply assign a new TU-local pointer
//  itself. This tiny registrar's constructor does that assignment instead, as
//  an ordinary member (see its use below, declared LAST so `this` is fully
//  built when it runs) -- a normal RAII side effect, not a language trick.
//  Definition (the TU-local pointer + the 3 consumers): cObserver.cpp, this
//  wave.
//
//  SAFETY -- the registered pointer is NEVER cleared on destruction (the
//  destructor body is ALSO existing/untouched Wave 1 content). In real
//  production this matches golden exactly (one TfObserver, created once, torn
//  down only at process exit -- golden's own `fObserver` global has the
//  identical lifetime assumption). In a test that constructs more than one
//  TfObserver, or that destroys one before calling
//  IniRecordMonitoringIndexCycleTime/RecordIndexAirOnTime1/RecordIndexAirOnTime2,
//  the pointer would dangle -- tests/test_observer_core.cpp (this wave) is
//  written to only call those 3 functions while its one `TfObserver` instance
//  is still in scope, precisely to avoid that.
// ===========================================================================
class TfObserver;

// ============================================================================
// AI(W906-FW-ObsSwap) 20260818: the three memo stand-ins MOVED VERBATIM from
// atester_shims.h (:342/:357-374 there) as part of retiring TfObserverShim --
// the live `fObserver` global is now backed by TfObserver (this facade), and
// the shim consumers (SCK_ART_Remainder's SaveTestSummarySECS whole-list
// assign, the Memo1 single-element peek) keep compiling against these exact
// names and shapes. Full provenance comments preserved from the shim header.
// ============================================================================
struct TfObserverMemoLines0 { AnsiString Strings0; };      // golden TMemo*->Lines->Strings[0] (only index used)

// golden cObserver.h:339 `TMyMemo *memoLotSummary;` -- SckArtRem_SaveTestSummarySECS
// does `fObserver->memoLotSummary->Lines=sList;` (golden SCK_ART.cpp:1965, a
// WHOLE-LIST assignment). Golden's TStrings::operator=(TPersistent*) COPIES --
// proven by golden itself calling `sList->Clear(); delete sList;` right after
// (SCK_ART.cpp:1967-1968) -- so this is a vector-backed COPY target.
struct TfObserverMemoLotSummaryLines
{
    std::vector<AnsiString> Strings;             // last-assigned COPY of golden Lines's content
    TfObserverMemoLotSummaryLines &operator=(TStringList *src)
    {
        Strings.clear();
        if(src!=0)
        {
            for(int i=0; i<src->Count; i++)
                Strings.push_back(src->Strings[i]);
        }
        return *this;
    }
};
struct TfObserverMemoLotSummary
{
    TfObserverMemoLotSummaryLines Lines;         // golden TMyMemo->Lines (TStrings*) -- whole-list-assign shape only
};
class W906Obs2_InstanceRegistrar
{
public:
    explicit W906Obs2_InstanceRegistrar(TfObserver *self);
};

// ===========================================================================
//  TfObserver -- non-VCL facade (golden cObserver.h, TfObserver:public TForm)
// ===========================================================================
class TfObserver
{
public:
    TfObserver();
    virtual ~TfObserver();

    // -- ctor-populated widget/data members Wave 1's methods dereference ----
    //    (golden cObserver.h line cited on each; grid/tray/chart/datetime
    //    members are allocated in the ctor's PORT-ONLY bootstrap section,
    //    then configured by the golden-translated ctor body -- see
    //    cObserver.cpp)
    TfObserverTray *mtRowA;              // golden cObserver.h:97  (TTMyTray*)
    TfObserverTray *mtRowB;              // golden cObserver.h:99  (TTMyTray*)
    TfObserverTray *mtRowC;              // golden cObserver.h:98  (TTMyTray*)
    TfObserverTray *mtRowD;              // golden cObserver.h:100 (TTMyTray*)
    TfObserverTray *mtCategorySum;       // golden cObserver.h:112 (TTMyTray*)
    TfObserverTray *mtRowName;           // golden cObserver.h:111 (TTMyTray*)
    TfObserverTray *mtNo;                // golden cObserver.h:130 (TTMyTray*)
    TfObserverTray *mtTotalName;         // golden cObserver.h:132 (TTMyTray*)
    TfObserverTray *myCategoryName;      // golden cObserver.h:113 (TTMyTray*)
    TfObserverTray *mtChName;            // golden cObserver.h:131 (TTMyTray*)
    TfObserverTray *mtDutName;           // golden cObserver.h:114 (TTMyTray*)
    TfObserverTray *mtArmName;           // golden cObserver.h:115 (TTMyTray*)
    TfObserverTray *mtCategoryNo;        // golden cObserver.h:121 (TTMyTray*)
    TfObserverTray *mtHeadTotal;         // golden cObserver.h:116 (TTMyTray*)
    TfObserverTray *mtSockTotal;         // golden cObserver.h:117 (TTMyTray*)
    TfObserverTray *mtPassHead;          // golden cObserver.h:118 (TTMyTray*)
    TfObserverTray *mtPassSocket;        // golden cObserver.h:119 (TTMyTray*)
    TfObserverTray *mtIfError;           // golden cObserver.h:120 (TTMyTray*)
    TfObserverTray *mtTotal;             // golden cObserver.h:134 (TTMyTray*)
    TfObserverTray *mtCategoryTotal;     // golden cObserver.h:133 (TTMyTray*)

    TfObserverGrid *StringGrid2;                 // golden cObserver.h:104 (TStringGrid*)
    TfObserverGrid *StringGrid3;                 // golden cObserver.h:108 (TStringGrid*)
    TfObserverGrid *StringGrid5;                 // golden cObserver.h:135 (TStringGrid*)
    TfObserverGrid *TimeInfoGrid;                // golden cObserver.h:55  (TStringGrid*)
    TfObserverGrid *strngrdTestTime;              // golden cObserver.h:292 (TStringGrid*)
    TfObserverGrid *sgTimeData;                   // golden cObserver.h:142 (TStringGrid*)
    TfObserverGrid *sg_ListTimeReceiveInfoGrid;   // golden cObserver.h:188 (TStringGrid*)
    TfObserverGrid *strngrdJamLog;                // golden cObserver.h:300 (TStringGrid*)
    TfObserverGrid *strngrdIndeAirOn1;            // golden cObserver.h:305 (TStringGrid*)
    TfObserverGrid *strngrdIndeAirOn2;            // golden cObserver.h:304 (TStringGrid*)
    TfObserverGrid *strngrdEventLog;              // golden cObserver.h:176 (TStringGrid*)
    TfObserverGrid *strngrdMDBQuery;              // golden cObserver.h:173 (TStringGrid*)

    TPanel *labMachineID;    // golden cObserver.h:378 (TPanel*)
    TPanel *labSerialNo;     // golden cObserver.h:375 (TPanel*)
    TPanel *pnlTotalCount;   // golden cObserver.h:172 (TPanel*)
    TPanel *lbltTotalLoader; // golden cObserver.h:162 (TPanel*)

    TComboBox *cbbTempChart;      // golden cObserver.h:413 (TComboBox*)
    TComboBox *cbDisplayData;     // golden cObserver.h:163 (TComboBox*)
    TComboBox *cbbEventLogYear;   // golden cObserver.h:178 (TComboBox*)
    TComboBox *cbbMonth;          // golden cObserver.h:181 (TComboBox*)
    TComboBox *cbbFilter;         // golden cObserver.h:185 (TComboBox*)

    TListBox *lstEventLog;        // golden cObserver.h:182 (TListBox*)

    TRadioGroup *rgContactCountKinds;         // golden cObserver.h:136 (TRadioGroup*)
    TRadioGroup *rgContactCountKindsForm;     // golden cObserver.h:137 (TRadioGroup*)
    TRadioGroup *rgContactCountHistory;       // golden cObserver.h:138 (TRadioGroup*)
    TRadioGroup *rgContactCountHistoryForm;   // golden cObserver.h:139 (TRadioGroup*)

    TfObserverChart *ChartYield;   // golden cObserver.h:51  (TChart*) -- 32 series pre-populated, PORT-ONLY (see .cpp)
    TfObserverChart *TempChart;    // golden cObserver.h:412 (TChart*) -- 0 series; golden ctor populates tcTotalCount of them
    TfObserverChart *Chart2;       // golden cObserver.h:174 (TChart*) -- 1 series pre-populated, PORT-ONLY (see .cpp)

    TfObserverDateTimePicker *DateTimePicker1;   // golden cObserver.h:164
    TfObserverDateTimePicker *DateTimePicker2;   // golden cObserver.h:165
    TfObserverDateTimePicker *DateTimePicker3;   // golden cObserver.h:166
    TfObserverDateTimePicker *DateTimePicker4;   // golden cObserver.h:167

    int iTotoalTestTime;   // golden cObserver.h:504 (int)  -- ctor sets 0
    int iShowYieldChart;   // golden cObserver.h:509 (int)  -- ctor sets 0

    // -- Wave 1 translated methods (bodies: cObserver.cpp, this wave) -------
    virtual void SetSiteYieldDiagram();                              // golden :771-837
    virtual void UpdateYieldChart();                                  // golden :838-864
    virtual void SpeedButton1Click(void *Sender);                     // golden :865-872
    virtual void UpdateBin();                                         // golden :873-904
    virtual void WriteContactKind();                                  // golden :1301-1648
    virtual void BtnQueryClick(void *Sender);                         // golden :2421-2776
    virtual void btReportClick(void *Sender);                         // golden :3186-3206
    virtual void rgContactCountKindsClick(void *Sender);              // golden :3219-3223
    virtual void rgContactCountHistoryClick(void *Sender);            // golden :3225-3229
    virtual void rgContactCountKindsFormClick(void *Sender);          // golden :3231-3235
    virtual void rgContactCountHistoryFormClick(void *Sender);        // golden :3237-3241
    virtual void lstEventLogClick(void *Sender);                      // golden :3757-3762
    virtual void cbbMonthChange(void *Sender);                        // golden :3763-3799
    virtual void GetEventLogText();                                   // golden :3801-3971
    virtual void btnQueryEventLogTxtClick(void *Sender);              // golden :4423-4426

    // -- DEVIATION: public+static, golden private+instance (see banner) -----
    static AnsiString CalculateStopTime(int Sec);          // golden :1686-1707
    static int        UnCalculateStopTime(AnsiString Time); // golden :2237-2253

    // -- PORT-ONLY testability seam (NOT golden methods) ---------------------
    // BtnQueryClick's WhereQuery/asQuery are local variables built, then
    // immediately handed to MyDBVProcess/MyDBVEventFreq -- both of which call
    // `sqlite3_get_table(dbReadOnly, ...)` UNCONDITIONALLY (cMyDB.cpp:1584),
    // with no NULL-handle guard. Calling BtnQueryClick for real from an
    // isolated unit test would therefore need a real, opened SQLite handle
    // (MyDBOpenDB(), which binds the production asDBPath) just to avoid a
    // NULL-dereference crash -- out of proportion, and out of this wave's
    // "don't mock cMyDB's behaviour, but don't need to RUN it either" brief.
    // These two static helpers extract the STRING ASSEMBLY BtnQueryClick does
    // (golden :2551-2556 and :2568-2569) into pure, no-DB-access functions
    // with IDENTICAL output -- BtnQueryClick (cObserver.cpp) calls them
    // internally instead of inlining the same sprintf, so this is a
    // zero-behaviour-change extraction, not new logic. Lets
    // tests/test_observer_core.cpp assert on exactly what SQL text the
    // Alarm_History eQueryType branch builds.
    static AnsiString BuildWhereQuery_OccurDateTimeRange(TDateTime d1, TDateTime t1, TDateTime d2, TDateTime t2); // golden :2551-2556
    static AnsiString BuildQuery_AlarmHistory(AnsiString WhereQuery);                                             // golden :2568-2569

    // -- GATE REGISTER (3): documented no-op stubs, bodies outside Wave 1 ---
    virtual void DoProduction_Summary_Report(AnsiString asStartData, AnsiString asStartTime,
                                              AnsiString asEndData, AnsiString asEndTime); // golden cObserver.h:552
    virtual void CountMTBF();                                                              // golden cObserver.h:517

    // NOTE: no `mtRow[MAX_SOCKET_ROW]` member here -- golden declares it as a
    // FILE-SCOPE global (cObserver.cpp:54 `TTMyTray *mtRow[MAX_SOCKET_ROW];`),
    // not a TfObserver class member, and this port keeps that shape (see
    // cObserver.cpp's own file-scope data section).

    // =========================================================================
    //  -- FW3-Obs2 ADD -- FW-3 cObserver Wave 2: OEE/statistics subset
    // =========================================================================
    //  AI(W906-FW3-Observer-W2) 20260818: additive-only continuation of Wave 1's
    //  facade (same file, same class -- Wave 1's own members/methods above are
    //  UNCHANGED). Golden ref unchanged: HT9011UC_Code_V3.33.906.0_20260618/
    //  cObserver.h + cObserver.cpp (see cObserver.cpp's own Wave 2 file-tail
    //  banner for the ABSENCE-CLAIM commands and U+FFFD count, not repeated here
    //  per Wave 1's own "don't duplicate across the two files" convention).
    //
    //  WAVE SCOPE (golden line span, ACTIVE / GATED / DEVIATION)
    //  -------------------------------------------------------------------------
    //    GetMachineData                      golden :755-767             ACTIVE
    //    Timer1Timer                         golden :708-753             ACTIVE, Close() -> facade no-op (see below)
    //    UpdateTempChart                     golden :905-943             ACTIVE
    //    cbbTempChartChange                  golden :945-948             ACTIVE
    //    ProcessRunInfo                      golden :2254-2331           ACTIVE
    //    RecordIndexTime                     golden :2814-2903           ACTIVE
    //    AddTimeData                         golden :2905-2966           ACTIVE
    //    RecordInArmTime                     golden :2968-2999           ACTIVE
    //    WriteCategoryData                   golden :3274-3547           ACTIVE
    //    CountMTBF                           golden :3683-3752           ACTIVE (Wave 1's no-op stub retired --
    //                                                                     SAME signature, declared already above,
    //                                                                     no header change; body only, cObserver.cpp)
    //    GetTimeDataText                     golden :4795-4839           ACTIVE
    //    RecordIndexCycle                    golden :4846-4887           ACTIVE
    //
    //    (free functions, declared at file scope below the class, matching
    //    golden cObserver.h:560-565's own placement)
    //    RecordStartTestTime()               golden :2136-2147           ACTIVE
    //    RecordEndTestTime(int)              golden :2150-2235           ACTIVE, 3 GATED callees (GATE REGISTER W2-1/2/3)
    //    RecordReceiveTestTime()             golden :4755-4764           ACTIVE (recon's :4755-4794 span also covers
    //                                                                     the NEXT function, pgcMessageChange -- NOT
    //                                                                     one of this wave's named targets, NOT
    //                                                                     translated; see cObserver.cpp for the
    //                                                                     verbatim golden text proving the boundary)
    //    IniRecordMonitoringIndexCycleTime() golden :1836-1844           ACTIVE
    //    RecordIndexAirOnTime1()             golden :5331-5343           ACTIVE
    //    RecordIndexAirOnTime2()             golden :5346-5358           ACTIVE
    //
    //  GATE REGISTER (W2)
    //  -------------------------------------------------------------------------
    //  (W2-1) RecordEndTestTime, golden :2176 `RecordMonitoringIndexCycleTime_New();`.
    //      Not declared ANYWHERE in the golden tree outside this one call site
    //      (grepped the cp950-decoded golden .cpp in full this wave -- 0 hits for
    //      a definition) and not one of this wave's named targets either way.
    //      TU-local no-op stub in cObserver.cpp, GATE-documented there.
    //  (W2-2) RecordEndTestTime, golden :2182 `RecordMonitoringIndexCycleTime();`.
    //      This one DOES have a real golden body (cObserver.cpp:1709-1751), but
    //      it is explicitly excluded from this wave's scope (task brief: "彈訊息" --
    //      it opens a message box on the outlier-count branch). Same TU-local
    //      no-op stub treatment, NOT a translation of the real body.
    //  (W2-3) RecordEndTestTime, golden :2168 `RecordTimeInfo();`. RecordTimeInfo's
    //      real body (golden :1846-2134, ~290 lines) is explicitly excluded this
    //      wave (task brief: "290 行未 recon 完"). Same TU-local no-op stub
    //      treatment.
    //  (W2-4) RecordEndTestTime, golden :2149/:2171 `extern int
    //      SendTestResultToHttp(); ... SendTestResultToHttp();`. Golden itself
    //      only ever forward-declares this LOCALLY (its real body lives in an
    //      unported networking/MES module) -- the SAME gap 3 sibling translators
    //      already solved (atester_32Site.cpp:394/397, aTester_Front.cpp:3264,
    //      aTester_Rear.cpp:3148): TU-local stub returning 1 (== upload OK) +
    //      `#define` shadow, so the call site keeps golden's own spelling.
    //  (W2-5) RecordEndTestTime, golden :2228 `fMain->slTestLog->AddText(str);`
    //      (inside the `IniConfig.bN28_SCK_OEE` / JSCK-OEE branch). `slTestLog`
    //      is NOT a forms/fMain.h facade member -- confirmed already gated for
    //      the SAME reason at cprod.cpp:2936-2939 (`fMain->slTestLog` inside a
    //      documented `#if 0` block, "missing facade member"), so this is the
    //      SAME pre-existing gap, not a new one, and out of this wave's write
    //      boundary (forms/fMain.h is not one of the 3 files this wave may
    //      touch). Everything ELSE in that branch (List1/List2 assembly,
    //      StringReplace) is real and ACTIVE; only this one trailing call is
    //      `#if 0`-gated in cObserver.cpp, with a pointer back to cprod.cpp's
    //      existing note.
    //
    //  DEVIATION -- Timer1Timer's Close()
    //  -------------------------------------------------------------------------
    //  golden :748 `Close();` (a TForm method that closes/hides the dialog after
    //  2 consecutive stale-Yield-Chart ticks). No TForm base here (same posture
    //  as every other translated form facade); modeled as a virtual no-op method
    //  below, matching Wave 1's own "GDI/window methods -> no-op" convention
    //  (TfObserverGrid::Repaint/Refresh). The COUNTING logic that decides WHEN to
    //  call it (iShowYieldChart/iCT/iSystemSec/iSystemMin state machine) is
    //  translated in full and is genuinely observable (iShowYieldChart itself is
    //  a Wave 1 member); only the actual window-close side effect is dropped.
    //
    //  NEW FACADE MEMBERS this wave's methods dereference (golden line cited)
    //  -------------------------------------------------------------------------
    //  ALL given in-class default member initializers (NSDMI): Wave 1's own
    //  ctor BODY (cObserver.cpp) is existing, untouched content this wave may
    //  not edit or move a line of (append-only rule) -- these new members are
    //  never listed in TfObserver()'s member-init-list either, so each one is
    //  constructed from ITS OWN in-class default before the ctor body runs,
    //  with NO change to that body. Same reasoning on the destructor side:
    //  Wave 1's ~TfObserver() body is also untouched, so pointer members added
    //  here are intentionally never `delete`d there -- a process-lifetime leak
    //  identical in kind (not new in kind) to the one already accepted for
    //  TfObserverChartSeries via TfObserverChart's OWN destructor loop, just
    //  without that loop's cleanup; acceptable because every consumer (real
    //  future UI wiring, and this wave's own tests) constructs at most one
    //  TfObserver per process lifetime.
    bool bShow = false;                            // golden cObserver.h:490 (bool)

    int RowNo = 0;                                 // golden cObserver.h:482 (private int; WriteCategoryData sets/reads
                                                    // it) -- promoted to public per this facade's established
                                                    // "no BCB6 private/__published split" convention (Wave 1 banner)

    TRadioGroup  *rgRowNo = new TRadioGroup();             // golden cObserver.h:129 -- WriteCategoryData
    TRadioButton *rbHeadNumber = new TRadioButton();       // golden cObserver.h:125
    TRadioButton *rbSocketNumber = new TRadioButton();     // golden cObserver.h:126
    TRadioButton *rbHeadPercent = new TRadioButton();      // golden cObserver.h:127
    TRadioButton *rbSocketPercent = new TRadioButton();    // golden cObserver.h:128

    TPanel *labPowerOnTime = new TPanel();          // golden cObserver.h:356 -- GetMachineData
    TPanel *labRunningTime = new TPanel();          // golden cObserver.h:357
    TPanel *labProductTime = new TPanel();          // golden cObserver.h:358
    TPanel *labLoadingCount = new TPanel();         // golden cObserver.h:359
    TPanel *labMUBA = new TPanel();                 // golden cObserver.h:360 -- ProcessRunInfo
    TPanel *labMTBA = new TPanel();                 // golden cObserver.h:361
    TPanel *labMTBF = new TPanel();                 // golden cObserver.h:362
    TPanel *pnlDayJamRate = new TPanel();           // golden cObserver.h:363

    // golden .dfm design-time ColCount/RowCount (tools/dfm2rc/ir_out/
    // cObserver.dfm.ir.json, read this wave): TimeInfoGrid_InArm ColCount=2
    // RowCount=15; strngrdTimeData ColCount=13, RowCount UNSET in the .dfm ->
    // default 5 (same "no explicit RowCount -> ctor default" posture Wave 1
    // already established for strngrdEventLog/strngrdMDBQuery).
    TfObserverGrid *TimeInfoGrid_InArm = new TfObserverGrid(2, 15);  // golden cObserver.h:140 -- RecordInArmTime
    TfObserverGrid *strngrdTimeData = new TfObserverGrid(13);        // golden cObserver.h:290 -- GetTimeDataText
    TListBox       *lstTimeData = new TListBox();                   // golden cObserver.h:288

    int iIndexCycleTimeCount = 0;                  // golden cObserver.h:540 -- IniRecordMonitoringIndexCycleTime
    int iPauseTime = 0;                            // golden cObserver.h:518 -- CountMTBF
    int iProductTime = 0;                          // golden cObserver.h:519
    int iJamTime = 0;                              // golden cObserver.h:520

    double dTempHistroy[tcTotalCount][60] = {};    // golden cObserver.h:522 -- UpdateTempChart
    double fRecordIndexTime[20] = {};              // golden cObserver.h:510 -- RecordIndexTime
    // NOTE: golden also declares `fRecordInArmTime1[20]`/`fRecordInArmTime2[20]`
    // (cObserver.h:512-513) but RecordInArmTime (this wave's only fRecordInArmTime*
    // consumer) reads/writes ONLY the plain `fRecordInArmTime[20]` array -- grepped
    // golden :2968-2999 in full, 0 hits for the 1/2 variants inside this range.
    // Not added (no delivered method touches them -- Wave 1's own "don't invent
    // surface" discipline).
    double fRecordInArmTime[20] = {};              // golden cObserver.h:511 -- RecordInArmTime

    // AI(W906-FW3-PIOEE) 20260828: golden cObserver.h:543 -- 同一批
    // KaiChen 20171127 OEE 欄位的第三個；下面 :547/:548 在 Wave 1 就補了，
    // 這一個當時因為「沒有交付的方法碰到」而留白（見 :796-797）。
    // TfProductionInfo::CalculateOEEReport（golden ProductionInfo.cpp:805）讀它，所以現在補上。
    // ⚠ MEASURED BEHAVIOUR NOTE：**本 port 沒有任何東西寫它**
    //   ——golden 的寫入點（cObserver.cpp:2067/:2071、uLotInfo.cpp:1793/:2155）全未翻。
    //   所以 OEE CSV 的 Test-Receive-Time 欄位會恆為 ""，直到其中一個寫入點落地。
    //   這是「忠實翻譯一個目前沒有生產者的欄位」，**不是 stub**。
    AnsiString sTestReceiveTime;                   // golden cObserver.h:543

    bool   bTestIndexZ = false;                    // golden cObserver.h:544 -- RecordIndexTime
    int    iTestIndexZCount = 0;                   // golden cObserver.h:545
    double dRecordIndexZTime[10] = {};             // golden cObserver.h:546
    AnsiString sTestIndexZTime;                    // golden cObserver.h:547 (AnsiString self-defaults to "")
    double dOEEIndexCycleTime = 0.0;               // golden cObserver.h:548

    TQPF_Timer tRecordInArmTimer;                  // golden cObserver.h:549 -- RecordInArmTime (real ctor, myTimer.h)

    AnsiString sRecordIndexCycleTime[200];         // golden cObserver.h:551 -- RecordIndexCycle (each self-defaults to "")

    // -- Wave 2 translated methods (bodies: cObserver.cpp, this wave) --------
    virtual void GetMachineData();                                 // golden :755-767
    virtual void Timer1Timer(void *Sender);                        // golden :708-753
    virtual void UpdateTempChart();                                // golden :905-943
    virtual void cbbTempChartChange(void *Sender);                 // golden :945-948
    virtual void ProcessRunInfo();                                 // golden :2254-2331
    virtual void RecordIndexTime(double fData);                    // golden :2814-2903
    virtual void AddTimeData(int iRow, double Time);               // golden :2905-2966
    virtual void RecordInArmTime();                                // golden :2968-2999
    virtual void WriteCategoryData();                              // golden :3274-3547
    virtual void GetTimeDataText();                                // golden :4795-4839
    virtual void RecordIndexCycle(bool bReset = false);            // golden :4846-4887

    // -- DEVIATION: TForm::Close() has no window here -- no-op (see banner) --
    virtual void Close();

    // ========================================================================
    // AI(W906-FW-ObsSwap) 20260818: swap-enablement members -- everything the
    // live fObserver consumers deref that Waves 1/2 had not yet carried
    // (measured usage sweep across all *.cpp, DEVLOG FW-ObsSwap). Same
    // unified-TPanel idiom the retired shim already justified (only ->Caption
    // is touched; the old "golden TLabel*" note was re-read and corrected to
    // TPanel* back in W7-F2).
    // ========================================================================
    TPanel *labModel    = new TPanel();   // golden cObserver.h:374
    TPanel *labFactory  = new TPanel();   // golden cObserver.h:377
    // AI(W906-FW-Q5) 20260818: golden cObserver.h:296 `TLabel *labLoaderCount;`
    // -- TPanel stand-in per this facade's labModel/labFactory precedent
    // (Caption is the only member the translated code touches).
    TPanel *labLoaderCount = new TPanel();
    TPanel *labBundleID = new TPanel();   // golden cObserver.h (SET_BUNDLE_INFO surface, uHGem G30-G33)
    TPanel *labBundlIn  = new TPanel();
    TPanel *labBundOut  = new TPanel();
    TPanel *lbSerialNumber01 = new TPanel();   // uHGem serial/firmware surface
    TPanel *lbSerialNumber02 = new TPanel();
    TPanel *lbSerialNumber03 = new TPanel();
    TPanel *lbSerialNumber04 = new TPanel();
    TPanel *lbFirmwareNumber01 = new TPanel();
    TPanel *lbFirmwareNumber02 = new TPanel();
    TPanel *lbFirmwareNumber03 = new TPanel();
    TPanel *lbFirmwareNumber04 = new TPanel();
    TfObserverMemoLines0    *Memo1Lines     = new TfObserverMemoLines0();     // golden TMemo* Memo1 (peek shape)
    TfObserverMemoLotSummary *memoLotSummary = new TfObserverMemoLotSummary(); // golden cObserver.h:339
    // AI(W906-FW-Q5) 20260818: StatisticalJamCount family REAL BODIES landed
    // (user-approved queue item 5). File writes go through the
    // W906_EVENTLOG_ROOT call-time getenv redirect (cObserver.cpp:1200
    // precedent); production (env unset) keeps golden's own literals.
    // golden decls cObserver.h:553-556 (+ ReadLoaderCount :555, btnSG_*
    // handlers :475-476).
    virtual void StatisticalJamCount(bool bIsNextDay=false);         // golden :5060-5277
    virtual void StatisticalLoaderCount();                           // golden :5278-5288
    virtual void ReadLoaderCount();                                  // golden :5289-5299
    virtual bool StatisticalJamCountEnable(AnsiString asJamCode);    // golden :5301-5329
    virtual void btnSG_QueryNowClick(TObject *Sender);               // golden :5361-5364 (body translated, NOT wired)
    virtual void btnSG_QueryYesterdayClick(TObject *Sender);         // golden :5366-5369 (body translated, NOT wired)

    // ========================================================================
    // AI(W906-FW3-Observer-W2) 20260825: Wave 3's 58 methods.
    //
    // These declarations are GENERATED FROM THE DEFINITIONS the three chunks
    // delivered, not transcribed from the method lists the chunks wrote for
    // themselves -- a list can drift from the code beside it, a generator
    // cannot. The golden ranges were then re-measured directly out of golden
    // (locate `TfObserver::<name>(`, brace-match to the closing `}`): 19 of the
    // 58 ranges the chunks cited were wrong, including two that both pointed at
    // WriteCategoryData's :3274-3547 and one that made ShowVer look like a
    // 3-line function when it is 81. The ranges below are the measured ones.
    //
    // `void *Sender` (not `TObject *Sender`) is this facade's convention -- the
    // parameter is kept, named, wherever golden has it, so a future wiring
    // layer has something to pass; see this file's own Wave-2 declarations.
    // ========================================================================
    // -- Wave 3 chunk A (display: form lifecycle + grid drawing), 13 methods
    virtual void FormShow(void *Sender);                                           // golden :347-652
    virtual void FormClose(void *Sender);                                          // golden :654-674
    virtual void FormDestroy(void *Sender);                                        // golden :676-695
    virtual void BtnExitClick(void *Sender);                                       // golden :697-706
    virtual void StringGrid2DrawCell(void *Sender, int ACol, int ARow);            // golden :952-984
    virtual void StringGrid3DrawCell(void *Sender, int ACol, int ARow);            // golden :988-1019
    virtual void DrawCenterLine(int Mode, int iLeft, int iCellWidth);              // golden :1021-1076
    virtual void DrawCellCounter(int iCol, int iRow, int iLeft, int iCellWidth);   // golden :1080-1132
    virtual void DrawCellCategory(int iCol, int iRow, int Mode);                   // golden :1136-1299
    virtual void StringGrid2MouseDown(void *Sender);                               // golden :1649-1654
    virtual void StringGrid3MouseDown(void *Sender);                               // golden :1658-1663
    virtual void rgRowNoClick(void *Sender);                                       // golden :1665-1668
    virtual void StringGrid5DrawCell(void *Sender, int ACol, int ARow);            // golden :1672-1682

    // -- Wave 3 chunk B (Precaution / MajorMaintenance records), 28 methods
    virtual bool CheckKeyInPrecautionMemoInformation(int iType);      // golden :3973-4035
    virtual void LoadPrecautionMenu();                                // golden :4037-4076
    virtual void SavePrecautionMemoInformation();                     // golden :4078-4152
    virtual void SavePrecautionParameter();                           // golden :4154-4183
    virtual void LoadPrecautionParameter();                           // golden :4185-4225
    virtual void LoadMajorMaintenanceMenu();                          // golden :4227-4261
    virtual void LoadPrecautionLogMenu();                             // golden :4263-4285
    virtual void LoadMajorMaintenanceLogMenu();                       // golden :4287-4309
    virtual void SaveMajorMaintenanceInformation();                   // golden :4311-4389
    virtual bool CheckKeyInMajorMaintenanceInformation(int iType);    // golden :4391-4421
    virtual void sbScreenkeyboardClick(void *Sender);                 // golden :4431-4450
    virtual void cobNoteContentsSetClick(void *Sender);               // golden :4452-4458
    virtual void sbHandlerPrecautionRecordSetClick(void *Sender);     // golden :4460-4465
    virtual void sbHandlerPrecautionRecordClearClick(void *Sender);   // golden :4467-4471
    virtual void sbHandlerPrecautionFormShowClick(void *Sender);      // golden :4473-4492
    virtual void sbPrecautionSaveClick(void *Sender);                 // golden :4494-4521
    virtual void sbPRFinishDateClick(void *Sender);                   // golden :4523-4527
    virtual void sbPRStartDateClick(void *Sender);                    // golden :4529-4533
    virtual void sbMajorMaintenanceDateClick(void *Sender);           // golden :4535-4538
    virtual void sbMajorMaintenanceStartTimeClick(void *Sender);      // golden :4540-4544
    virtual void sbUndesirablePhenomenonClick(void *Sender);          // golden :4546-4550
    virtual void sbCountermeasureClick(void *Sender);                 // golden :4552-4556
    virtual void sbUndesirablePhenomenonClearClick(void *Sender);     // golden :4558-4562
    virtual void sbCountermeasureClearClick(void *Sender);            // golden :4564-4567
    virtual void sbMajorMaintenanceEndTimeClick(void *Sender);        // golden :4569-4572
    virtual void sbMajorMaintenanceSaveClick(void *Sender);           // golden :4574-4594
    virtual void sbMajorMaintenanceSearchClick(void *Sender);         // golden :4596-4678
    virtual void sbSearchPrecautionLogClick(void *Sender);            // golden :4680-4753

    // -- Wave 3 chunk C (event log / save / misc handlers), 17 methods
    virtual void pgcObservChange(void *Sender);               // golden :2335-2389
    virtual void BtnSaveClick(void *Sender);                  // golden :2393-2419
    virtual void DateTimePicker1CloseUp(void *Sender);        // golden :2777-2784
    virtual void Image1DblClick(void *Sender);                // golden :2786-2812
    virtual void bAutoSaveEventLog(bool flag);                // golden :3001-3170
    virtual void btAutoSaveClick(void *Sender);               // golden :3172-3175
    // AI(W906-FW-SIG-W15) 20260826: 回填 golden 完整簽章（GATE (C-log-6) 退役）。
    virtual void lbltTotalLoaderMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);   // golden :3177-3184
    virtual void edYieldMaxClick(void *Sender);               // golden :3207-3211
    virtual void edYieldMinClick(void *Sender);               // golden :3213-3217
    virtual void mtRowAMouseUp(void *Sender, int X, int Y);   // golden :3243-3272
    virtual void ShowVer();                                   // golden :3549-3629
    virtual void btOpenLoadLogClick(void *Sender);            // golden :3638-3681
    virtual void pgcMessageChange(void *Sender);              // golden :4766-4793
    virtual void lstTimeDataClick(void *Sender);              // golden :4841-4844
    virtual void btnBackupLogYearClick(void *Sender);         // golden :5371-5391
    virtual void btnClearTimeClick(void *Sender);             // golden :5393-5399
    virtual void btnLot1Click(void *Sender);                  // golden :5401-5424

    // ========================================================================
    // AI(W906-FW3-Observer-W2) 20260825: TAB SURFACE + Precaution-record state.
    //
    // WHY NOW, AND WHY THESE. Wave 3's three translation chunks arrived with
    // gates FW3A-2 / FW3A-5 / FW3A-6 covering all of this. Re-measuring the
    // gate PREMISES during integration found two different failures:
    //
    //   FW3A-2 claimed `rg "TTabSheet|TPageControl" -g '*.h' vclcompat/ forms/`
    //   returned 0 hits. That is simply false -- vclcompat/Controls.h:478/:487
    //   have carried both classes since commit 1a74870 (W7-A1 + W7-F0), long
    //   before this wave. What was genuinely missing was the MEMBERS, below.
    //
    //   FW3A-5/FW3A-6 claimed the Precaution-record members and the five
    //   Load*/Save* methods exist nowhere. True when each chunk agent ran, and
    //   stale by the time they landed: chunk B defines LoadPrecautionMenu,
    //   LoadMajorMaintenanceMenu, LoadPrecautionLogMenu,
    //   LoadMajorMaintenanceLogMenu and SavePrecautionParameter; chunk C
    //   defines ShowVer and reads bIsLoaded on 10 lines. Wave 1's own ctor note
    //   (cObserver.cpp:497-503) deferred these members to "the wave that
    //   translates the Precautions tab" -- this is that wave.
    //
    // NSDMI throughout, for the same reason the block above states: Wave 1's
    // ctor body is append-only content this wave does not reorder.
    //
    // NOT a member: golden's `bool bTabVisible[20]` is FILE-SCOPE in golden
    // cObserver.cpp:57, not a class member -- it lands as a TU-local static in
    // cObserver.cpp beside bShowYieldAll, matching this port's handling of
    // golden's other file-scope arrays. (The gate text called it
    // `bTabVisible[8]`; only indices 0..7 are ever used, but the declaration is
    // 20 wide and is translated at its real width.)
    // ========================================================================
    TTabSheet *tsScanner                = new TTabSheet();   // golden cObserver.h:36
    TTabSheet *tsCounter                = new TTabSheet();   // golden cObserver.h:34
    TTabSheet *tsTestCate               = new TTabSheet();   // golden cObserver.h:35
    TTabSheet *tsMDBQuery               = new TTabSheet();   // golden cObserver.h:37
    TTabSheet *tsYield                  = new TTabSheet();   // golden cObserver.h:50
    TTabSheet *tsTestInfo               = new TTabSheet();   // golden cObserver.h:52
    TTabSheet *tsTemperature            = new TTabSheet();   // golden cObserver.h:57
    TTabSheet *tsMDB                    = new TTabSheet();   // golden cObserver.h:153
    TTabSheet *tsOEE_ProductionInfor    = new TTabSheet();   // golden cObserver.h:186
    TTabSheet *tsDataRecord             = new TTabSheet();   // golden cObserver.h:189
    TTabSheet *tsPrecautionsRecord      = new TTabSheet();   // golden cObserver.h:191
    TTabSheet *tsHanderMajorMaintenance = new TTabSheet();   // golden cObserver.h:225
    TTabSheet *tsPrecautionLog          = new TTabSheet();   // golden cObserver.h:258
    TTabSheet *tsIndexAirOn1            = new TTabSheet();   // golden cObserver.h:302
    TTabSheet *tsIndexAirOn2            = new TTabSheet();   // golden cObserver.h:303
    TTabSheet *tsLotInfo                = new TTabSheet();   // golden cObserver.h:306

    // pgcObserv/pgcPrecautions need ->ActivePage (golden :608/:671/:2384) so they
    // take the TfObserverPageControl subclass; the other two only ever touch
    // ->ActivePageIndex, so they stay on the plain vclcompat type -- the same
    // mixed usage forms/fLotInfo.h:1078 already established.
    TfObserverPageControl *pgcObserv      = new TfObserverPageControl();  // golden cObserver.h:33
    TfObserverPageControl *pgcPrecautions = new TfObserverPageControl();  // golden cObserver.h:190
    TPageControl          *pgcTestInfo    = new TPageControl();           // golden cObserver.h:53
    TPageControl          *pgcMessage     = new TPageControl();           // golden cObserver.h:152

    bool bChangeRow[2] = {false, false};           // golden cObserver.h:483
    bool bIsLoaded[13] = {};                       // golden cObserver.h:491
    bool bSavePrecautionRecordFinish = false;      // golden cObserver.h:523
    bool bStartPrecautionRecord = false;           // golden cObserver.h:524
    bool bChangeReciepeSaveMajorMaintenanceRecord = false;   // golden cObserver.h:525
    bool bShowMajorMaintenanceRecord = false;      // golden cObserver.h:526
    AnsiString asStartPrecautionRecordMOId;        // golden cObserver.h:527 (self-defaults to "")

    // ========================================================================
    // AI(W906-FW3-Observer-W2) 20260825: the 68 widget members chunks B and C
    // dereference. Derived MECHANICALLY, not from either chunk's own list: the
    // merged file was compiled, every "was not declared in this scope" name
    // collected, and each looked up in golden cObserver.h for its real VCL type
    // (the golden line is cited on every line below). Nothing here is a guess
    // about what the form "probably has".
    //
    // WHERE THE LINE IS. This wave adds a member when a LIVE translated
    // statement dereferences it -- i.e. when the tree does not compile without
    // it. It does NOT add members whose only call sites sit inside a gate: the
    // 31 caption sinks behind GATE FW3A-4 (counted, not estimated -- every one
    // was looked up in golden cObserver.h and has a type there: 22 TPanel, 4
    // TRadioButton, 2 TButton, 1 TCheckBox, 1 TGroupBox, 1 TLabel; namely
    // labDeviceName, APHeadLabel13/14/18,
    // labReleaseDate, Button7, btAutoSave, CheckBox1, grpATCSerialNumber,
    // labDayJamRate, RadioButton17..20, the 17 SPIL pal* panels) are equally
    // addable and are deliberately QUEUED, because adding them means un-gating
    // as well, which is a decision with its own acceptance gate rather than a
    // side effect of making this merge build.
    //
    // Type mapping: golden TPanel/TEdit/TMemo/TComboBox/TSpeedButton/TButton go
    // to the vclcompat classes of the same name; golden TFileListBox to
    // vclcompat::TFileListBox; golden TDateTimePicker to this file's own
    // TfObserverDateTimePicker (the same stand-in DateTimePicker1..4 use).
    // NSDMI for the same reason as every block above -- Wave 1's ctor body is
    // append-only content this wave does not reorder.
    // ========================================================================
    // golden TPanel (33)
    TPanel                     *labVersion                       = new TPanel(); // golden cObserver.h:379
    TPanel                     *pnApprovedManager                = new TPanel(); // golden cObserver.h:265
    TPanel                     *pnCountermeasure                 = new TPanel(); // golden cObserver.h:249
    TPanel                     *pnDOCUMENTNO                     = new TPanel(); // golden cObserver.h:263
    TPanel                     *pnEndTime                        = new TPanel(); // golden cObserver.h:274
    TPanel                     *pnFinishName                     = new TPanel(); // golden cObserver.h:269
    TPanel                     *pnFinishType                     = new TPanel(); // golden cObserver.h:268
    TPanel                     *pnMMSpecificationNO              = new TPanel(); // golden cObserver.h:252
    TPanel                     *pnMajorMaintenanceCheckNo        = new TPanel(); // golden cObserver.h:242
    TPanel                     *pnMajorMaintenanceCheckPersonnel = new TPanel(); // golden cObserver.h:244
    TPanel                     *pnMajorMaintenanceClassType      = new TPanel(); // golden cObserver.h:238
    TPanel                     *pnMajorMaintenanceDate           = new TPanel(); // golden cObserver.h:237
    TPanel                     *pnMajorMaintenanceEndTime        = new TPanel(); // golden cObserver.h:241
    TPanel                     *pnMajorMaintenancePersonnel      = new TPanel(); // golden cObserver.h:243
    TPanel                     *pnMajorMaintenanceStartTime      = new TPanel(); // golden cObserver.h:240
    TPanel                     *pnNoteContents                   = new TPanel(); // golden cObserver.h:284
    TPanel                     *pnNoteLog                        = new TPanel(); // golden cObserver.h:264
    TPanel                     *pnPRSpecificationNO              = new TPanel(); // golden cObserver.h:224
    TPanel                     *pnPrecautionEndTime              = new TPanel(); // golden cObserver.h:218
    TPanel                     *pnPrecautionLogApprovedManager   = new TPanel(); // golden cObserver.h:276
    TPanel                     *pnPrecautionLogDocumentNo        = new TPanel(); // golden cObserver.h:271
    TPanel                     *pnPrecautionLogEndTime           = new TPanel(); // golden cObserver.h:280
    TPanel                     *pnPrecautionLogFinishName        = new TPanel(); // golden cObserver.h:270
    TPanel                     *pnPrecautionLogFinishType        = new TPanel(); // golden cObserver.h:278
    TPanel                     *pnPrecautionLogNoteContents      = new TPanel(); // golden cObserver.h:272
    TPanel                     *pnPrecautionLogPromptDay         = new TPanel(); // golden cObserver.h:267
    TPanel                     *pnPrecautionLogStartTime         = new TPanel(); // golden cObserver.h:275
    TPanel                     *pnPrecautionLogWatchmakers       = new TPanel(); // golden cObserver.h:277
    TPanel                     *pnPrecautionStartTime            = new TPanel(); // golden cObserver.h:213
    TPanel                     *pnPromptDay                      = new TPanel(); // golden cObserver.h:279
    TPanel                     *pnStartTime                      = new TPanel(); // golden cObserver.h:273
    TPanel                     *pnUndesirablePhenomenon          = new TPanel(); // golden cObserver.h:248
    TPanel                     *pnWatchmakers                    = new TPanel(); // golden cObserver.h:266

    // golden TEdit (11)
    TEdit                      *edApprovedManager                = new TEdit(); // golden cObserver.h:210
    TEdit                      *edFinishName                     = new TEdit(); // golden cObserver.h:216
    TEdit                      *edMajorMaintenanceCheckNo        = new TEdit(); // golden cObserver.h:245
    TEdit                      *edMajorMaintenanceCheckPersonnel = new TEdit(); // golden cObserver.h:247
    TEdit                      *edMajorMaintenancePersonnel      = new TEdit(); // golden cObserver.h:246
    TEdit                      *edNoteContents                   = new TEdit(); // golden cObserver.h:208
    TEdit                      *edPrecautionRecordDocumentNo     = new TEdit(); // golden cObserver.h:205
    TEdit                      *edPromptDay                      = new TEdit(); // golden cObserver.h:221
    TEdit                      *edWatchmakers                    = new TEdit(); // golden cObserver.h:212
    TEdit                      *edYieldMax                       = new TEdit(); // golden cObserver.h:60
    TEdit                      *edYieldMin                       = new TEdit(); // golden cObserver.h:61

    // golden TMemo (8)
    TMemo                      *Memo1                       = new TMemo();  // golden cObserver.h:380
    TMemo                      *Memo2                       = new TMemo();  // golden cObserver.h:144
    TMemo                      *Memo3                       = new TMemo();  // golden cObserver.h:147
    TMemo                      *Memo4                       = new TMemo();  // golden cObserver.h:148
    TMemo                      *MemoCountermeasure          = new TMemo();  // golden cObserver.h:251
    TMemo                      *MemoHandlerPrecautionRecord = new TMemo();  // golden cObserver.h:203
    TMemo                      *MemoNoteLog                 = new TMemo();  // golden cObserver.h:262
    TMemo                      *MemoUndesirablePhenomenon   = new TMemo();  // golden cObserver.h:250

    // golden TComboBox (8)
    TComboBox                  *cobCountermeasure            = new TComboBox(); // golden cObserver.h:254
    TComboBox                  *cobHandlerPrecautionRecord   = new TComboBox(); // golden cObserver.h:202
    TComboBox                  *cobMajorMaintenanceClassType = new TComboBox(); // golden cObserver.h:239
    TComboBox                  *cobMajorMaintenanceSearch    = new TComboBox(); // golden cObserver.h:256
    TComboBox                  *cobNoteContents              = new TComboBox(); // golden cObserver.h:219
    TComboBox                  *cobPRFinishType              = new TComboBox(); // golden cObserver.h:217
    TComboBox                  *cobSearchPrecautionLog       = new TComboBox(); // golden cObserver.h:282
    TComboBox                  *cobUndesirablePhenomenon     = new TComboBox(); // golden cObserver.h:253

    // golden TSpeedButton (3)
    TSpeedButton               *sbMajorMaintenanceDate      = new TSpeedButton(); // golden cObserver.h:227
    TSpeedButton               *sbMajorMaintenanceEndTime   = new TSpeedButton(); // golden cObserver.h:229
    TSpeedButton               *sbMajorMaintenanceStartTime = new TSpeedButton(); // golden cObserver.h:228

    // golden TButton (1)
    TButton                    *btnQueryEventLogTxt = new TButton();        // golden cObserver.h:183

    // golden TFileListBox (2)
    vclcompat::TFileListBox    *FileListBoxMajorMaintenance = new vclcompat::TFileListBox(); // golden cObserver.h:257
    vclcompat::TFileListBox    *FileListBoxPrecautionLog    = new vclcompat::TFileListBox(); // golden cObserver.h:283

    // golden TDateTimePicker (2)
    TfObserverDateTimePicker   *DateTimePickerEnd   = new TfObserverDateTimePicker(); // golden cObserver.h:222
    TfObserverDateTimePicker   *DateTimePickerStart = new TfObserverDateTimePicker(); // golden cObserver.h:223

    // ========================================================================
    // AI(W906-FW3-Observer-W3) 20260825: the caption-only widgets GATE (FW3A-4)
    // was holding, but ONLY the nine whose value source survives the port.
    //
    // FW3A-4 covered 31 widgets. Re-measuring them one family at a time before
    // adding anything (the value-provenance rule this project paid for in
    // FW-TAG1 -- "does the TYPE exist" is the easy half; "where does the VALUE
    // come from" is the half that bites) split them three ways:
    //
    //   ADDED HERE (9). Every one is written from something real and live:
    //     labDeviceName   <- GetLastOpenFN()          APHeadLabel18 <- MyDBQClearDT()
    //     labReleaseDate  <- RunInfo.SoftwareDate     Button7       <- !SystemStart
    //     APHeadLabel13/14 <- the DrawCell column totals the grid loops compute
    //     labDayJamRate / btAutoSave / CheckBox1 <- plain visibility writes
    //
    //   STILL GATED, value source absent (4): RadioButton17..20. golden READS
    //     ->Checked to pick RowNo (golden :1675-1678). Which radio starts
    //     checked is a `.dfm` design-time property and this port has no .dfm
    //     loader, so all four would read false and RowNo would silently keep
    //     its previous value instead of being selected. Same class as the
    //     VacuumUnit Tag dispatch -- see vclcompat/Controls.h's Tag note.
    //
    //   STILL GATED, right-hand side absent (17): the SPIL pal* panels. The
    //     gate text blamed the missing panels alone; that is INCOMPLETE. The
    //     values come from `fSCKART->sInfo_Customer` and 15 siblings, and
    //     forms/fSCKART.h's TfSCKART -- a deliberately measured subset -- has
    //     NO sInfo_* field at all.
    //       cmd: grep -n "sInfo_\|iInfo_MultiLotCnt" forms/fSCKART.h -> 0 hits (20260825)
    //     So adding the 17 panels would create 17 members with no possible
    //     writer. They wait on the SCK_ART.cpp completion wave, exactly like
    //     chunk C's C-log-11 (which needs the sInfoArr_* array siblings).
    //
    //   Also still gated for an unrelated reason: grpATCSerialNumber, whose
    //   branch compares against ATC_TYPE_31 (0 hits tree-wide).
    // ========================================================================
    TPanel      *labDeviceName      = new TPanel();        // golden cObserver.h:101
    TPanel      *APHeadLabel13      = new TPanel();        // golden cObserver.h:149
    TPanel      *APHeadLabel14      = new TPanel();        // golden cObserver.h:150
    TPanel      *APHeadLabel18      = new TPanel();        // golden cObserver.h:151
    TPanel      *labReleaseDate     = new TPanel();        // golden cObserver.h:376
    TButton     *Button7            = new TButton();       // golden cObserver.h:169
    TButton     *btAutoSave         = new TButton();       // golden cObserver.h:170
    TCheckBox   *CheckBox1          = new TCheckBox();     // golden cObserver.h:171
    TLabel      *labDayJamRate      = new TLabel();        // golden cObserver.h:354

    // -- PORT-ONLY, NOT a golden member -- see W906Obs2_InstanceRegistrar's
    //    banner above. Declared LAST so `this` is fully constructed (every
    //    member above it already initialized) when its ctor runs.
    W906Obs2_InstanceRegistrar _w906Obs2SelfRegister{this};
};

// AI(W906-FW-ObsSwap) 20260818: the integration call was made (user-approved
// queue, 20260818 morning): the live global is now backed by THIS facade.
// TfObserverShim is retired from atester_shims.h/.cpp; the global's
// DEFINITION moved home to cObserver.cpp (golden cObserver.h declares
// `extern PACKAGE TfObserver *fObserver;` -- same homecoming as B4/cMyDB).
// Static-init construction is safe ONLY because the ctor guards its config
// reads on INIFileGeneral being open (see cObserver.cpp ctor) -- golden
// constructs this form after OpenGeneralIniFile in WinMain order, and the
// guard reproduces that precondition instead of crashing on a NULL ini or
// seeding production files at static-init (the Gerneral.ini incident class).
extern TfObserver *fObserver;                    // golden cObserver.h (extern PACKAGE)

// AI(W906-FW3-Observer-W2) 20260818: free functions (golden cObserver.h:
// 560-565, same file-scope placement -- these are NOT TfObserver members in
// golden either). Bodies: cObserver.cpp, this wave, appended at file tail.
void RecordStartTestTime();                     // golden :2136-2147
int  RecordEndTestTime(int iArm);               // golden :2150-2235 (0:arm1 1:arm2 2:雙Arm)
void RecordReceiveTestTime();                   // golden :4755-4764
void IniRecordMonitoringIndexCycleTime();       // golden :1836-1844
void RecordIndexAirOnTime1();                   // golden :5331-5343
void RecordIndexAirOnTime2();                   // golden :5346-5358

// AI(W906-OBSWEB) 20260925: web export of the form (cObserver.cpp tail) --
// wb_serve WS `observer.get` -> web/page/Data.Observer.html.
// act = open|timer|tab|rowNo|form|year|month|file|filter|query (see the
// banner above the definition for the golden event each one replays).
// Caller holds ht9045::formjson::FormLock.  Throws std::invalid_argument.
#include <string>
std::string W906_ObserverJson(const std::string &act, int arg, const std::string &text);

#endif // FORMS_FOBSERVER_H
