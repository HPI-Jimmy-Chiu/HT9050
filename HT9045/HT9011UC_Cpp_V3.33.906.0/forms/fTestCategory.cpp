// =============================================================================
//  forms/fTestCategory.cpp  --  definitions for the fTestCategory facade
//
//  Steven 20260925 (Data.TestCategory) -- AI(W906-TCAT-WEB)：GATE (T-0c)a／(T-1)..(T-7)
//  解開。T-1..T-7 的本體搬到根目錄 cTestCategory.cpp（ht9045_sm，因為 IsNNMode）；本檔
//  只留 ht9045_globals 就夠的部分：建構子（golden :20-29，MyStringGD／Tag 迴圈現在是真的）、
//  InitCateCell、SetShowCateMode、ShowTestCategory、FormClose，以及 TfTestCategoryGrid::
//  Refresh()（VCL 重畫 → OnDrawCell，見 forms/fTestCategory.h 的 D-3 段）。T-8 FormDestroy
//  仍是 `#if 0`。下面 20260828 的 banner 是史料。
//
//  AI(W906-FW3-BTQ1) 20260828: new file, FW wave FW3-BTQ1 (2 of 5 facades).
//  GOLDEN SOURCE: HT9011UC_Code_V3.33.906.0_20260618/cTestCategory.cpp
//  (547 lines) + cTestCategory.h (46 lines), read with
//  `io.open(p, encoding='cp950')`.
//  SPAN: 13 golden `TfTestCategory::` member bodies, 518 span lines
//  (tools/census/wave_preflight.py, 20260828).
//
//  THIS WAVE, against the 13-member denominator:
//     4 ACTIVE           InitCateCell / SetShowCateMode / ShowTestCategory /
//                        FormClose
//     1 ACTIVE-PARTIAL   the ctor -- golden :26-29 live, :20-25 + :30-32 gated
//     8 GATED-WITH-BODY  464 golden span lines carried as `#if 0` transcript
//  BY LINES: 54 of 518 really live (10.4%).  FIVE of the eight gates fall on
//  ONE symbol -- `IsNNMode()`, ht9045_sm only.
//
//  See forms/fTestCategory.h for the full GATE REGISTER (T-1)..(T-8), the
//  ACTIVE evidence, DEVIATIONS D-1..D-8, the FIELD LIST (including MyStringGD/
//  PtrDC/pCanvas deliberately NOT declared) and the ZERO-WRITER notes about
//  `Active`, `bShow` and the two arrays.
//
//  ⚠ EVERY `#if 0` BLOCK BELOW HAS NEVER BEEN COMPILED.  The text is golden's
//  own (the only edits are dropping `__fastcall`, the unused FormClose
//  parameters and sgArm1DrawCell's un-portable TRect&/TGridDrawState), so it is
//  a faithful TRANSCRIPT -- not verified code.  Most of the identifiers it
//  names (MyStringGD, PtrDC, pCanvas, TCanvas, GetDC, ReleaseDC, MyDrawText,
//  TFontStyles, fsBold, ArmStr, IsNNMode, MAX_SOCKET_ROW, TestIF, LastSet,
//  iTestBinCount, TestSocket, asBarCodeErrorSend, LogSoftwareOffTime, ...) are
//  NOT members of this facade and/or have no reachable definition; un-gating
//  requires supplying them first.
//
//  BACKSLASH-COMMENT SCAN (the -Wcomment line-splice trap): cTestCategory.cpp
//  was scanned 20260828 for a `//` comment whose line ends in a backslash.
//  ZERO hits -- so no comment delimiter was changed anywhere in this file.
// =============================================================================
#include "forms/fTestCategory.h"
#include "Config.h"              // IniConfig.iShowCateByArm (Config.h:731, Config.cpp, ht9045_globals)
#include "vclcompat/LedCore.h"   // vclcompat's guarded TColor / cl* block -- clWhite (:61)

// AI(W906-FW3-BTQ1) 20260828: `clWhite` only.  Deliberately NOT
// `using vclcompat::TColor;` -- cmydef.h:16 already puts `typedef int TColor`
// in the global namespace, and this file uses THAT one for the ColorPtr member
// so the facade's declared type matches every existing consumer of TColor in
// the tree.  Same single-name import shape as forms/fTrayMapping.cpp:58-60.
using vclcompat::clWhite;
using vclcompat::clBtnFace;   // Steven 20260925: TfTestCategoryGrid::Refresh default FixedColor

// Steven 20260925 (Data.TestCategory): golden Graphics.hpp `clWindow` =
// (TColor)(COLOR_WINDOW | 0x80000000) -- TStringGrid's default Color (the dfm does
// not override it).  vclcompat has no clWindow constant (grep vclcompat/ -- 0 hits,
// 20260925), so it is spelled once here with the same system-colour encoding
// vclcompat uses for clBtnFace (0x8000000F).
static const TColor W906_clWindow = static_cast<TColor>(0x80000005u);

// AI(W906-FW3-BTQ1) 20260828: TfTestCategory/fTestCategory were FREE tree-wide
// -- same idiom as forms/fCounterSel.cpp:38 / forms/fLd_ULd.cpp:43.  The ACTIVE
// part of golden's ctor writes only four of this object's own scalars, so this
// static-init `new` touches no global -- no SIOF risk (docs/KNOWLEDGE.md
// "static-init ctor 不可碰 NULL 全域"; the fLaserSensor incident that rule
// comes from turned 88 of 134 ctest binaries into SEGFAULTs).  Golden's
// remaining ctor statements call GetDC() on a window handle and `new TCanvas`;
// both are gated -- see GATE (T-0c) below.
TfTestCategory *fTestCategory = new TfTestCategory();

// ---------------------------------------------------------------------------
//  golden :17-33.  ACTIVE-PARTIAL: golden :26-29 are live below; golden :20-25
//  (the MyStringGD/Tag loop) and :30-32 (GetDC / new TCanvas) are GATE (T-0c)
//  and appear as transcript further down this file.
//  Steven 20260925: SUPERSEDED -- :20-25 live below, :30-32 modelled in
//  cTestCategory.cpp; the transcript further down is gone.
// ---------------------------------------------------------------------------
TfTestCategory::TfTestCategory()
{
    //Steven 20260925 (Data.TestCategory): GATE (T-0c)a LIFTED -- golden :20-25.
    // TfTestCategoryGrid now carries Tag and MyStringGD is declared.  Own-field
    // writes only (sgArm1/sgArm2 are this object's own default-member `new`s,
    // already run before this body), so the static-init SIOF rule still holds.
    TfTestCategoryGrid *TempPtr[]={sgArm1, sgArm2};
    for(int i=0; i<2; i++)
    {
        MyStringGD[i]=TempPtr[i];
        MyStringGD[i]->Tag=i;
    }
    EdgeWidth=80;
    EdgeHeight=24;
    bCateByArm=false;
    bShow=false;
    // golden :30-32, `PtrDC[0]=GetDC(sgArm1->Handle);` `PtrDC[1]=GetDC(sgArm2->Handle);`
    // `pCanvas=new TCanvas;` -- no window / HDC / TCanvas here.  Steven 20260925:
    // modelled in ROOT cTestCategory.cpp (TU-local PtrDC[] = grid index, pCanvas =
    // capture canvas); the binding of OnDrawCell (dfm) is W906_TestCategoryInstall().
}

// ---------------------------------------------------------------------------
//  Steven 20260925 (Data.TestCategory): TfTestCategoryGrid::Refresh -- golden
//  TStringGrid::Refresh -> WM_PAINT -> TCustomGrid.Paint.  See the D-3 block in
//  forms/fTestCategory.h for the order and the one omitted VCL detail.
// ---------------------------------------------------------------------------
void TfTestCategoryGrid::Refresh()
{
    const int rows = RowCount;
    const int cols = ColCount;
    const int fr = FixedRows;
    const int fc = FixedCols;
    W906_PaintRows = rows;
    W906_PaintCols = cols;
    W906_Paint.assign(static_cast<std::size_t>(rows > 0 && cols > 0 ? rows * cols : 0), W906_CellPaint());
    ++W906_RefreshCount;

    // (1) VCL DefaultDrawing: FillRect with FixedColor (fixed cells) or Color, then
    //     TStringGrid.DrawCell's TextRect of Cells[ACol][ARow] -- this form never
    //     writes Cells, so the text is always "".  The grid's own Font (dfm Batang,
    //     Style=[]) -> Bold=false.
    for(int r=0; r<rows; r++)
        for(int c=0; c<cols; c++)
        {
            W906_CellPaint *p = W906_Cell(c, r);
            p->Text  = "";
            p->Brush = (r<fr || c<fc) ? clBtnFace : W906_clWindow;
            p->Bold  = false;
            p->Drawn = false;
        }

    if(!OnDrawCell)
        return;

    // (2) OnDrawCell, in TCustomGrid.Paint's four DrawCells passes.
    for(int r=0; r<fr && r<rows; r++)            // corner
        for(int c=0; c<fc && c<cols; c++)
            OnDrawCell(this, c, r);
    for(int r=0; r<fr && r<rows; r++)            // fixed rows x scrolling cols
        for(int c=fc; c<cols; c++)
            OnDrawCell(this, c, r);
    for(int r=fr; r<rows; r++)                   // fixed cols x scrolling rows
        for(int c=0; c<fc && c<cols; c++)
            OnDrawCell(this, c, r);
    for(int r=fr; r<rows; r++)                   // body
        for(int c=fc; c<cols; c++)
            OnDrawCell(this, c, r);
}

// ---------------------------------------------------------------------------
//  golden :496-509.  ACTIVE.  Pure triple loop over this object's own arrays;
//  the only external names are clWhite (vclcompat/LedCore.h:61, header-only
//  const) and TColor (cmydef.h:16).  No IsNNMode -- which is precisely why
//  this one is ACTIVE while its four array siblings are GATE (T-4)..(T-7).
// ---------------------------------------------------------------------------
void TfTestCategory::InitCateCell()
{
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<4; j++)
        {
            for(int k=0; k<8; k++)
            {
                ColorPtr[i][j][k]=clWhite;
                TestResult[i][j][k]=-1;
            }
        }
    }
}

// ---------------------------------------------------------------------------
//  golden :511-525.  ACTIVE.
//  ⚠ `DEBUG_WIN7_FULL_HD` is NOT defined anywhere in this tree (0 hits in
//  CMakeLists.txt / MachineDefine.h / MachineType.h, 20260828), so the `#else`
//  arm is what compiles.  Golden's `#ifdef` is kept verbatim rather than
//  folded away -- a build that defines it must still get golden's 247/143.
//  ⚠ golden addresses itself through the GLOBAL (`fTestCategory->Height`)
//  rather than through `this`.  Kept verbatim; at the one call chain that
//  reaches here they are the same object.
// ---------------------------------------------------------------------------
void TfTestCategory::SetShowCateMode()
{
    bCateByArm=(IniConfig.iShowCateByArm!=0);
    #ifdef DEBUG_WIN7_FULL_HD
        if(bCateByArm)
            fTestCategory->Height=247;
        else
            fTestCategory->Height=143;
    #else
        if(bCateByArm)
            fTestCategory->Height=192;
        else
            fTestCategory->Height=105;
    #endif
}

// ---------------------------------------------------------------------------
//  golden :448-463.  ACTIVE.
//  ⚠ `Refresh()` is DEVIATION D-3, an offline no-op: no window, no OnDrawCell.
//  This does NOT repaint anything -- it preserves golden's control flow and
//  which grid golden would have repainted.
// ---------------------------------------------------------------------------
void TfTestCategory::ShowTestCategory(int iIndex)
{
    SetShowCateMode();
    if(bUseTwoArm32Site==true)
    {
        sgArm1->Refresh();
        sgArm2->Refresh();
    }
    else
    {
        if(iIndex==0 || bCateByArm==false)
            sgArm1->Refresh();
        else
            sgArm2->Refresh();
    }
}

// ---------------------------------------------------------------------------
void TfTestCategory::FormClose()   // golden :527-531, DEVIATION D-7
{
    bShow=false;
}

// ===========================================================================
//  GATE REGISTER -- translated golden bodies, deliberately NOT COMPILED.
//  See forms/fTestCategory.h for the per-entry reasoning.
// ===========================================================================

// Steven 20260925 (Data.TestCategory): the `#if 0` transcripts GATE (T-0c) and
// (T-1)..(T-7) that stood here are GONE from this file, not deleted from the
// port: (T-0c)a is live in the ctor above; (T-0c)b (PtrDC/pCanvas) is modelled
// in ROOT cTestCategory.cpp; (T-1) FormShow, (T-2) AdjFormData, (T-3)
// sgArm1DrawCell and (T-4)..(T-7) the four array accessors are LIVE golden text
// in ROOT cTestCategory.cpp (ht9045_sm -- IsNNMode, cinitial.cpp:7417).  Keeping
// a second copy here would be two texts of one golden body drifting apart.
// Only (T-8) remains gated below.

#if 0 // GATE (T-8) FormDestroy -- golden :533-546.  TWO GATES:
      // (a) LINK -- LogSoftwareOffTime(AnsiString) is declared cmydef.h:5032
      //     but its ONLY definition is acarry_shims.cpp:255 (a `{}` no-op) and
      //     that file is in ht9045_sm; it is not one of the four sanctioned
      //     forms->sm exceptions.
      // (b) MISSING FIELDS -- PtrDC[]/pCanvas are not declared (DEVIATION D-4)
      //     and ReleaseDC needs a real HDC.
      // (MyDBIProcess IS reachable -- 2-arg body aHotPlateSubstrate.cpp:1099,
      // a sanctioned exception -- so it is NOT a blocker.)
void TfTestCategory::FormDestroy(TObject *Sender)
{
    try
    {
        for(int i=0; i<2; i++)
            ReleaseDC(0, PtrDC[i]);
        delete pCanvas;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "TfTestCategory::FormDestroy");
    }
    LogSoftwareOffTime("TfTestCategory, FormDestroy");                          //Steven 20210526 : 紀錄軟體執行時間
}
#endif // GATE (T-8)
