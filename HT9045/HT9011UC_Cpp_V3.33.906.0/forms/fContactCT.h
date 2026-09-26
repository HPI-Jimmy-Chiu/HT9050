// =============================================================================
//  forms/fContactCT.h  --  non-VCL facade for golden's TfContactCT (cContactCT.h)
//
//  AI(W906-FW3-ContactCT-WA) 20260818: new file, FW-3 queue item 2.
//  Golden ref: HT9011UC_Code_V3.33.906.0_20260618/cContactCT.h (53 lines) +
//  cContactCT.cpp (1,330 lines, cp950/Big5 -- decoded with `python3 -c
//  "open(path,'rb').read().decode('cp950')"`, 0 U+FFFD, this wave).
//
//  ROLE
//  ----
//  TfContactCT is golden's "Contact Counter Kinds" (Contact CT) dialog: the
//  per-site/per-arm yield/count grid (sgYield) plus its Count-Clear button.
//  It is the direct data source for 25 of fYieldMonitoring's 39 GATE touches
//  (see forms/fYieldMonitoring.h GATE REGISTER (Y1)) and cMyDB.cpp's own
//  pre-existing `TODO(GA1-B4): fContactCT not yet ported` (cMyDB.cpp:645-647,
//  `fContactCT->SaveSiteYield(sAction)`).  This wave translates the WHOLE
//  golden .cpp file -- unlike cShowBinSelect (see forms/fShowBinSelect.h),
//  cContactCT's dependency surface turned out to be fully resolvable this
//  wave (every non-widget symbol it touches already exists in the port tree
//  -- see ABSENCE-CLAIM section in cContactCT.cpp), so no method needed to be
//  deferred.
//
//  THIS IS A NEW CLASS -- NO PRIOR SEAM TO DISPLACE
//  --------------------------------------------------------------------------
//  Unlike fObserver/fYieldMonitoring, there was no pre-existing TfContactCT
//  stand-in of ANY shape anywhere in the tree before this wave (verified:
//  `grep -rn "class.*TfContactCT\|fContactCT *;" --include=*.h .` -- 0 hits,
//  20260818, over the whole port tree excluding golden/build dirs). Two
//  TU-local partial seams DO exist and are noted for the main loop's
//  homecoming pass (see forms/fYieldMonitoring.h GATE REGISTER (Y1)):
//  csystem.cpp:4840-4850's `W7C1_TfContactCTSeam` (file-scope macro'd,
//  invisible to other TUs, covers only rgYieldType/ClearData(int,int) -- a
//  strict subset of this facade). This header does NOT reuse or displace it;
//  that reconciliation is a main-loop integration decision.
//
//  WAVE SCOPE (every golden method, golden line span -- ALL ACTIVE this wave
//  except the two per-statement GATEs called out below)
//  --------------------------------------------------------------------------
//    TfContactCT()                golden :25-31    ACTIVE, DEVIATION (GetDC dropped, see below)
//    FormShow                     golden :33-75    ACTIVE
//    FormClose                    golden :77-81    ACTIVE
//    FormDestroy                  golden :83-95    ACTIVE, DEVIATION (ReleaseDC/pCanvas dropped)
//    ShowFormComp                 golden :97-146   ACTIVE
//    sgYieldDrawCell               golden :148-307  DEVIATION signature (see below); MyDrawText -> W906_Draw capture (C1 lifted 20260925)
//    TestSite2X2Mode               golden :309-315  ACTIVE (pure)
//    ReturnSiteData                golden :317-518  ACTIVE (pure)
//    ReturnSiteDataArray           golden :523-591  ACTIVE (pure)
//    ReturnSiteDataArray_AutoClean golden :593-645  ACTIVE (pure)
//    GetLowYield_AutoClean         golden :647-722  ACTIVE (pure)
//    rgYieldTypeClick              golden :724-738  ACTIVE
//    sgYieldDblClick               golden :740-886  ACTIVE, 1 GATE (Application->MessageBox)
//    ClearData                     golden :888-920  ACTIVE
//    ClearData_AutoClean           golden :922-935  ACTIVE
//    sgYieldMouseDown              golden :937-942  ACTIVE, DEVIATION signature (Button/Shift dropped)
//    btClearCountClick             golden :944-1069 ACTIVE, 3 GATEs (fSecurity x2, Application->MessageBox x1)
//    btYieldChartClick             golden :1071-1075 ACTIVE
//    SaveSiteYield                 golden :1077-1236 ACTIVE (file I/O -- see WRITE-PATH NOTE)
//    ACSmartClearData              golden :1238-1280 ACTIVE
//    SaveTotalYield                golden :1282-1328 ACTIVE (file I/O -- see WRITE-PATH NOTE)
//
//  DEVIATION -- GDI members dropped entirely (hDC/pCanvas)
//  --------------------------------------------------------------------------
//  golden's `HDC hDC; TCanvas *pCanvas;` (cContactCT.h:32-33) exist ONLY to
//  paint sgYield's cells (sgYieldDrawCell) and are acquired/released in the
//  ctor/FormDestroy (`GetDC(sgYield->Handle)` / `ReleaseDC(0,hDC); delete
//  pCanvas;`).  vclcompat has NO TCanvas anywhere in the tree (`grep -rn
//  "class TCanvas" vclcompat/` -- 0 hits, 20260818) -- this is the SAME
//  "GDI methods not this wave" gap common.h's own file head already
//  documents for MyDrawText (`#if 0 // TODO(wave-canvas): MyDrawText (6
//  overloads, TCanvas/TRect/TColor VCL GDI)`, common.h:387-393).  Rather than
//  invent a TCanvas stand-in (out of this wave's "don't build UI infra"
//  brief, and MyDrawText -- the only thing that would ever call it -- is
//  itself gated), hDC/pCanvas are NOT declared here at all, and the two
//  lines that touch them (ctor's GetDC, FormDestroy's ReleaseDC+delete) are
//  dropped with a pointer-back comment at each site (cContactCT.cpp).  No
//  other method reads hDC/pCanvas outside sgYieldDrawCell.
//
//  DEVIATION -- sgYieldDrawCell / sgYieldMouseDown signatures trimmed
//  --------------------------------------------------------------------------
//  golden: `sgYieldDrawCell(TObject*, int ACol, int ARow, TRect &Rect,
//  TGridDrawState State)` and `sgYieldMouseDown(TObject*, TMouseButton
//  Button, TShiftState Shift, int X, int Y)`.  `TRect`, `TGridDrawState`,
//  `TMouseButton` and `TShiftState` do not exist anywhere in the tree
//  (`grep -rn "class TRect\|TGridDrawState\|TMouseButton\|TShiftState"` --
//  0 hits, 20260818) -- they are GDI/input-event VCL types with no port
//  equivalent, and neither parameter is read by anything other than the
//  (fully GATED, see below) MyDrawText calls / the harmless X/Y mouse
//  coordinates.  Kept ONLY the parameters the pure logic actually uses
//  (ACol/ARow; X/Y), dropped the rest.  This is the same class of forced
//  signature trim as e.g. forms/fObserver.h's UpdateBin (golden TObject*
//  Sender dropped where genuinely unused) -- not a behaviour change, since
//  the dropped parameters are never read by anything this wave translates.
//
//  GATE REGISTER
//  --------------------------------------------------------------------------
//  (C1) sgYieldDrawCell's ~15 `MyDrawText(pCanvas, ...)` calls (golden
//       :176/:181/:189/:193/:203/:217/:226/:228/:243/:248/:259/:261/:282/
//       :296/:297) -- MyDrawText itself is `#if 0`-gated tree-wide
//       (common.h:387-393, "TODO(wave-canvas)"), so these cannot even be
//       named.  Every SURROUNDING statement (iShowSiteYieldIndex/str/SCT
//       computation, AYield.sprintf, ASE_Yield[n]+= accumulation) is real
//       and ACTIVE -- only the actual paint call is gated, matching
//       forms/fObserver.h GATE (2)'s "only the opaque-type call is gated"
//       precedent. `pCanvas->Font->Color=clBlack;` lines are gated alongside
//       (same missing TCanvas).
//       AI(W906-CTWEB) 20260925: (C1) LIFTED.  MyDrawText -> W906_Draw
//       (capture buffer W906_Paint[][], see class body), Font->Color ->
//       W906_FontColor; the whole-block gates are golden statements again.
//       Consumer: W906_ContactCTJson() (cContactCT.cpp tail) -> wb_serve WS
//       `contactct.get` -> web/page/Data.ContactCT.html.  The golden line
//       list above has one typo: the clInfoBk/clRed call is :295, not :296.
//  (C2) sgYieldDblClick's `Application->MessageBox(...)==IDYES` guard
//       (golden :853) and btClearCountClick's non-KYEC confirm dialog
//       (golden :1001) -- vclcompat has no TApplication/MB_YESNO/IDYES
//       surface anywhere (confirmed precedent: database.cpp:444-447's
//       identical `TODO(GA1-B6)` gate, Public/HTEditList.cpp's GATE (1)).
//       SAME safe default as GA1-B6: without a real prompt we cannot assume
//       the operator would answer Yes, so the confirmed-destructive branch
//       (bClear=true's ClearALLCT block / bClearData=true's ClearALLCT
//       block) is NOT taken -- i.e. the gated code behaves as if the
//       operator dismissed/cancelled the dialog.  This is a conservative
//       (never destructively clears data the user did not get to confirm),
//       reviewable default, not a guess.
//  (C3) btClearCountClick's `fSecurity->Insufficient(107)` (golden
//       :952/:985) -- fSecurity has NO facade anywhere in the tree
//       (`grep -rn "class TfSecurity" --include=*.h .` -- 0 hits, 20260818;
//       same absence forms/fObserver.h's own GATE (1) already documents).
//       Golden's `Insufficient(iType)` returns TRUE when access IS
//       sufficient (confirmed by reading golden cSecurity.cpp:573-597 in
//       full this wave -- the name is misleading but the body is
//       unambiguous: `if(AccessLevel<LevelSet.AccessLevel[iType]) return
//       false;` then `return true;`).  Safe default: FAIL CLOSED --
//       substitute `false` (treat as "insufficient" / permission NOT
//       verified) at both call sites, so the surrounding `==false) return;`
//       early-exit UNCONDITIONALLY fires when `bRefreshFunction==false`
//       (CC_ASE_KaohSiung branch) and ALWAYS fires for every other customer
//       code's `else if` branch.  This means btClearCountClick's body below
//       that point is, for now, reachable ONLY via the CC_KYEC_LEE branch
//       (which never touches fSecurity) or when `bRefreshFunction==true`
//       (a real, already-ported global) -- documented, not silently
//       narrowed: the translated body below is complete and will start
//       executing for every customer the moment a real fSecurity facade
//       lands and this gate is lifted.
//  (C4) btClearCountClick's CC_KYEC_LEE branch touches `fMain->
//       bHasCleanCount`, `fMain->cbUserSelect`, `fMain->stOperatorClick`,
//       `fMain->btLogin`, `fMain->spbUserName` -- grepped forms/fMain.h in
//       full this wave: 0 hits for any of the five (fMain.h HAS
//       ChangeLevelAttr already, real+ACTIVE, so that one call is NOT
//       gated).  The whole CC_KYEC_LEE `if/else` block up to and including
//       the `if(AccessLevel<iDefSupervisorLevel) return;` guard is `#if 0`
//       (one block, five missing members, not five separate gates) --
//       documented GAP, same idiom as (C1)-(C3).
//
//  WRITE-PATH NOTE -- SaveSiteYield / SaveTotalYield are FILE WRITES, left
//  ACTIVE (not gated)
//  --------------------------------------------------------------------------
//  Both call golden's already-ACTIVE, already-tree-wide `WriteDataToFile`/
//  `MyForceDirectories` (common.h:337-341, "un-gated ... bodies translated"
//  per that header's own banner) against `asYieldRecordPath` -- a per-
//  install config path (common.h:111, NOT `D:\HT9045\system\...` literal),
//  read-then-append CSV logging, structurally identical in kind to
//  cObserver.cpp's already-ACTIVE WriteCategoryData (FW-3 cObserver Wave 2).
//  Neither writes machine state, teach data, or lastdata.dat (the two paths
//  KNOWLEDGE.md's "前提死掉不代表答案就是退役" entry warns about) -- they are
//  pure yield-history CSV appenders, gated only by golden's own
//  `IniConfig.bI29EnableYieldRecord` / `bI29YieldRecordIntervalIC` flags
//  (default off).  Left ACTIVE per this wave's brief ("讀 golden 本體決定");
//  main loop should still confirm `asYieldRecordPath` resolves under
//  `--dry`/test harnesses before any live run touches it.
//
//  DESIGN NOTE -- three facade-only widget wrapper shapes
//  --------------------------------------------------------------------------
//  TfContactCTGrid : public vclcompat::TStringGrid
//    vclcompat::TStringGrid (built for SECS SV/EC grids) deliberately
//    carries ONLY Cells[][]/RowCount/ColCount (its own file-head SCOPE note:
//    "NO rendering, NO FixedRows/.../ColWidths").  golden's sgYield ALSO
//    needs ->Top/->Width/->Height (ShowFormComp/FormShow layout math) and
//    ->RowHeights[i]= (ShowFormComp) and ->MouseToCell(x,y,&col,&row)
//    (sgYieldDblClick) and ->Refresh() (ClearData/ACSmartClearData/
//    btClearCountClick/rgYieldTypeClick).  Same "compose, don't fork the
//    shared header" posture as forms/fObserver.h's TfObserverGrid: Top/
//    Width/Height/RowHeights[] are COSMETIC LAYOUT ONLY and offline INERT
//    (nothing reads them back), and MouseToCell/Refresh are documented
//    GDI/mouse-hit-testing no-ops (GATE (C1) sibling -- see class comment).
//    AI(W906-VclGrid-1) 20260820: the 5-subclass TStringGrid-extension
//    consolidation wave (vclcompat/StringGrid.h) moved Visible/Font/
//    DefaultColWidth/FixedRows/FixedCols/Row/ColWidths[]/ClearRow(i) from
//    forms/fObserver.h's TfObserverGrid and forms/fConfiguration.h's
//    TfConfigurationGrid to the shared base -- TfContactCTGrid was READ in
//    full as part of that wave's survey and is DELIBERATELY LEFT AS-IS: its
//    Top/Width/Height/RowHeights[]/MouseToCell/Refresh set shares no member
//    NAME with that union (RowHeights indexes ROWS, not columns, and this
//    class has no Visible/Font/DefaultColWidth/FixedRows/FixedCols/Row/
//    ColWidths/ClearRow of its own to begin with), so there is nothing here
//    to shadow or collapse. TfContactCTGrid still inherits the new base
//    members for free (unused, harmless) same as every other TStringGrid
//    subclass in the tree.
//  TfContactCTRadioGroup : public vclcompat::TRadioGroup
//    Adds ->Height/->Columns (FormShow layout math), same cosmetic-inert
//    rationale.
//  TfContactCTButton : public vclcompat::TButton
//    Adds ->Name (btClearCountClick's `Ptr->Name=="btClearCount"` check --
//    golden casts the Sender to `TSpeedButton*` even though the .h declares
//    `TButton *btClearCount;`; BCB6 VCL controls share ->Name at the
//    TComponent root so this compiles and works in golden even though the
//    cast target class is technically wrong -- preserved verbatim, cast
//    dropped since this facade's Name lives directly on the TButton-derived
//    type).
//
//  HYDRATION (dfm IR: tools/dfm2rc/ir_out/cContactCT.dfm.ir.json, this wave)
//  --------------------------------------------------------------------------
//    sgYield       : ColCount=4, RowCount=9 (dfm design-time default --
//                    ShowFormComp() overwrites RowCount at runtime per test
//                    mode, exactly as golden does), Top=121, Width=361,
//                    Height=168.
//    rgYieldType   : Columns=3, Height=88 (dfm default; FormShow() overwrites
//                    to 50/70 depending on CosFunction flags, exactly as
//                    golden does).
//  palClearCnt/btClearCount/btYieldChart carry no facade-only geometry field
//  (vclcompat::TPanel/TButton have no Top/Left/Width/Height slot and no
//  method this wave reads one back), so no hydration was needed for them.
// =============================================================================
#ifndef FORMS_FCONTACTCT_H
#define FORMS_FCONTACTCT_H

#include "vclcompat/vcl_compat.h"   // AnsiString, TObject
#include "vclcompat/Controls.h"     // TRadioGroup, TPanel, TButton
#include "vclcompat/StringGrid.h"   // TStringGrid

using vclcompat::TObject;
using vclcompat::TStringGrid;
using vclcompat::TRadioGroup;
using vclcompat::TPanel;
using vclcompat::TButton;

// -- facade-only widget extensions (see DESIGN NOTE above) -------------------
class TfContactCTGrid : public vclcompat::TStringGrid
{
public:
    int Top, Width, Height;         // cosmetic layout only, offline inert
    int RowHeights[64];              // golden ->RowHeights[i]= (ShowFormComp);
                                      // 64 covers the largest RowCount this
                                      // file ever assigns (33, _32Site4X8M)

    explicit TfContactCTGrid(int initialColCount = 4, int initialRowCount = 9)
        : vclcompat::TStringGrid(initialColCount, initialRowCount),
          Top(121), Width(361), Height(168)
    {
        for (int i = 0; i < 64; ++i) RowHeights[i] = 0;
    }

    // GATE (C1) sibling: GDI/mouse-hit-testing no-ops, see banner.
    void MouseToCell(int /*X*/, int /*Y*/, int &ACol, int &ARow) { ACol = 0; ARow = 0; }
    void Refresh() {}
};

class TfContactCTRadioGroup : public vclcompat::TRadioGroup
{
public:
    int Height;
    int Columns;
    TfContactCTRadioGroup() : Height(88), Columns(3) {}
};

class TfContactCTButton : public vclcompat::TButton
{
public:
    AnsiString Name;
};

class TfContactCTPanel : public vclcompat::TPanel
{
public:
    int Height;   // cosmetic layout only, offline inert -- FormShow reads
                   // palClearCnt->Height (golden :51/:74/:136/:142); no such
                   // field exists on vclcompat::TPanel.
    TfContactCTPanel() : Height(33) {}   // dfm design-time default (ir_out/cContactCT.dfm.ir.json)
};

// =============================================================================
//  TfContactCT -- non-VCL facade (golden cContactCT.h)
// =============================================================================
class TfContactCT
{
public:
    // -- widgets (golden __published, cContactCT.h:15-19) --------------------
    TfContactCTGrid       *sgYield       = new TfContactCTGrid();
    TfContactCTRadioGroup *rgYieldType   = new TfContactCTRadioGroup();
    TfContactCTPanel       *palClearCnt   = new TfContactCTPanel();
    TfContactCTButton     *btClearCount  = new TfContactCTButton();
    TButton                *btYieldChart  = new TButton();

    // -- data (golden cContactCT.h:40) ----------------------------------------
    bool bShow;

    // AI(W906-CTWEB) 20260925: GATE (C1) lifted into a CAPTURE BUFFER (Steven
    // 20260925 task: Data.ContactCT.html shows the real grid).  golden paints
    // sgYield through its own TCanvas (pCanvas bound to GetDC(sgYield->Handle),
    // golden :29-30) with MyDrawText (golden common.cpp:1366-1379: set Brush
    // [+Font] colour, FillRect, DrawText centred).  The port has no TCanvas, so
    // every MyDrawText in sgYieldDrawCell now calls W906_Draw(), which records
    // {text, brush colour, font colour} for that (ACol, ARow).  Colours are the
    // golden clXxx NAMES (the page maps them; W906_ContactCTJson ships the RGB
    // table used).  W906_FontColor models pCanvas->Font->Color, which PERSISTS
    // between cells in golden: the 4-arg MyDrawText overload (label cells) does
    // not set it, so a label cell inherits whatever the previous cell left
    // (TFont default clWindowText, or clBlack after a data cell's
    // `pCanvas->Font->Color=clBlack;`).  Cells golden never MyDrawText's keep
    // drawn=false; W906_ContactCTJson fills them with VCL's default cell paint.
    struct W906_CellPaint { AnsiString text; AnsiString bg; AnsiString fg; bool drawn = false; };
    enum { W906_MAX_ROWS = 64, W906_MAX_COLS = 4 };      // 64 = RowHeights[] bound above
    W906_CellPaint W906_Paint[W906_MAX_ROWS][W906_MAX_COLS];
    AnsiString W906_FontColor = "clWindowText";         // TFont default Color
    bool W906_FontBold = false;                          // golden FormShow :39 pCanvas->Font->Style=fsBold
    int  W906_FontSize = 0;                              // golden FormShow :40 pCanvas->Font->Size=8
    int  Height = 289;                                   // golden TForm->Height (dfm ClientHeight=289 as
                                                         // the pre-FormShow value); written by FormShow
                                                         // :74 / ShowFormComp :136/:142, now un-gated
    // fg == nullptr -> golden's 4-arg MyDrawText(pCanvas,Rect,str,BrushColor)
    // (font colour untouched); otherwise the 5-arg overload.
    void W906_Draw(int ACol, int ARow, const AnsiString &str, const char *bg, const char *fg = nullptr);

    // NOTE on the form's own geometry (golden TForm->Height, ShowFormComp
    // :136/:142, FormShow :74): no facade field is declared for it -- nothing
    // in this wave's translated code, nor any test, ever READS it back after
    // ShowFormComp/FormShow write it (verified: the statements are the
    // faithful `Height=...;` assignment to the golden form's own inherited
    // TForm->Height, which this facade class does not model at all, same
    // "form-level geometry not modelled" posture as forms/fYieldMonitoring.h
    // and forms/fObserver.h -- neither declares Left/Top/Width/Height on the
    // form object itself). Each site is translated as a comment-documented
    // no-op assignment (see cContactCT.cpp) rather than silently dropped.

    TfContactCT();

    void FormShow(TObject *Sender);
    void FormClose(TObject *Sender);
    void FormDestroy(TObject *Sender);
    void ShowFormComp();

    // DEVIATION: signature trimmed, see banner ("sgYieldDrawCell /
    // sgYieldMouseDown signatures trimmed").
    void sgYieldDrawCell(int ACol, int ARow);

    bool TestSite2X2Mode();
    AnsiString ReturnSiteData(int Arm, int ARow);
    double ReturnSiteDataArray(bool bType, int i, int j);
    int ReturnSiteDataArray_AutoClean(bool bType, int i, int j);
    double GetLowYield_AutoClean(int iType);   // iType: 0 LowYield, 1 SiteYieldDifferent

    void rgYieldTypeClick(TObject *Sender);
    void sgYieldDblClick(TObject *Sender);
    void ClearData(int iRow, int iCol);
    void ClearData_AutoClean();

    // DEVIATION: signature trimmed, see banner.
    void sgYieldMouseDown(int X, int Y);

    void btClearCountClick(TObject *Sender);
    void btYieldChartClick(TObject *Sender);

    void SaveSiteYield(AnsiString SaveEvent);
    void ACSmartClearData();
    void SaveTotalYield(AnsiString SaveEvent);   // golden :1282, "Sam 20231106: record Total yield"
};

// AI(W906-FW3-ContactCT-WA) 20260818: global NOT defined here -- main-loop
// homecoming (same posture as forms/fYieldMonitoring.h / forms/fObserver.h
// before their own swap waves). This facade's ctor is a plain field
// bootstrap (no config/file I/O, no INI read) -- see cContactCT.cpp's ctor
// comment -- so static-init safety is NOT the blocker for defining a live
// global from this header; the blocker is purely "no code anywhere
// currently constructs one", same as fYieldMonitoring pre-swap.
extern TfContactCT *fContactCT;   // golden cContactCT.h:51 (extern PACKAGE TfContactCT *fContactCT;)

// AI(W906-CTWEB) 20260925: web export of the form (cContactCT.cpp tail).
// yieldType < 0 = open the window (FormShow); n = click rgYieldType item n.
// Caller holds ht9045::formjson::FormLock.  Throws std::invalid_argument.
#include <string>
std::string W906_ContactCTJson(int yieldType);

#endif // FORMS_FCONTACTCT_H
