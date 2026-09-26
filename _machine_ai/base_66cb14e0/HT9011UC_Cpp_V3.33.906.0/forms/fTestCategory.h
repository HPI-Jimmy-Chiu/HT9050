// =============================================================================
//  forms/fTestCategory.h  --  non-VCL facade for golden's TfTestCategory
//                             (cTestCategory.h)
//
//  AI(W906-FW3-BTQ1) 20260828: new file, FW wave FW3-BTQ1 (2 of 5 facades).
//  GOLDEN SOURCE: HT9011UC_Code_V3.33.906.0_20260618/cTestCategory.h (46 lines)
//  + cTestCategory.cpp (547 lines), read with `io.open(p, encoding='cp950')`
//  (0 U+FFFD), written out UTF-8 / bare-LF.
//
//  ROLE
//  ----
//  TfTestCategory is golden's per-site bin display: two TStringGrids (one per
//  arm) whose cells are painted from a private `TestResult[2][4][8]` /
//  `ColorPtr[2][4][8]` pair.  The tester engine pushes results in through
//  SetTestCateCell*/SetTestingCateCell and reads them back out through
//  GetTestResult; everything else is layout.
//
//  `python tools/census/wave_preflight.py cTestCategory.cpp` (20260828, re-run
//  at closeout): 13 bodies = 13 `TfTestCategory::` members + 0 file-scope,
//  518 span lines, 0 brace-swallow short-fall, 0 overload collapse.
//
//  THIS WAVE, against the 13-member denominator:
//     4 ACTIVE           InitCateCell / SetShowCateMode / ShowTestCategory /
//                        FormClose
//     1 ACTIVE-PARTIAL   the ctor -- 4 of its 17 golden lines are live
//     8 GATED-WITH-BODY  FormShow / AdjFormData / sgArm1DrawCell /
//                        SetTestCateCellINT / SetTestCateCellAS /
//                        SetTestingCateCell / GetTestResult / FormDestroy
//  BY LINES, counting only what is really live: 54 of 518 golden span lines
//  (10.4%).  Counting the PARTIAL ctor whole would say 67 (12.9%) -- the
//  honest figure is 54.  Quote both numbers or neither.
//  ⚠ This is a THIN wave BY LINK BOUNDARY, not by omission: five of the eight
//  gates (AdjFormData + the four array accessors) fall on ONE symbol,
//  `IsNNMode()`, whose only two non-test definitions are both in ht9045_sm.
//  See GATE (T-2).
//
//  ============================================================================
//  *** THE NAMES `TfTestCategory` / `fTestCategory` ARE FREE -- THIS WAVE
//  CLAIMS THEM ***
//  ============================================================================
//  Measured 20260828 with `python tools/census/wave_preflight.py
//  cTestCategory.cpp` (section [3], 1203 port files scanned): (a) class name
//  `clear`, (c) global name `clear`, (d) TU-local seam `clear`.  (b) reported 5
//  same-NAME members of OTHER classes (FormShow/FormClose on TfCounterClear /
//  TfStartCondition / TfCounterSel) -- different classes, not an occupation.
//  Cross-checked with `rg -c -g '!build_*' -g '*.h' -g '*.cpp'
//  "\bTfTestCategory\b" .`: every hit is a COMMENT or a `#if 0` line
//  (atester_ProcessCount.cpp, Command.cpp, Automation/auto9045.cpp) except the
//  generated tools/dfm2rc/layout_out/cTestCategory_layout.gen.cpp, where it is
//  a STRING LITERAL in a .dfm widget table -- and `rg -n "layout_out"
//  CMakeLists.txt` returns 0, i.e. that file is in NO add_library.
//
//  ⚠ THIS WAVE FALSIFIES THREE OF THE TREE'S OWN COMMENTS.  Recorded here, NOT
//  edited (this wave adds files only):
//    * atester_ProcessCount.cpp:1479 says "`fTestCategory` has 0 declarations
//      in the tree".  True when written; false after this file lands.
//    * Command.cpp:9175 quotes a 0-hit grep for
//      `TfTestCategory \*fTestCategory\|extern.*fTestCategory`.  Same.
//    * Command.cpp:15247-15249 (item A2) says "no class, no global".  Same.
//  NONE of them is code: all three sit above `#if 0` blocks that stay `#if 0`.
//  Nothing starts compiling because of this wave.
//
//  ============================================================================
//  CONDITION-TWO CHECK (does a port of these bodies ALREADY exist, possibly as
//  a RENAMED FREE FUNCTION outside the class?)
//  ============================================================================
//  Searched 20260828, whole port tree, build_* excluded.
//  ⚠ MEASUREMENT CONVENTION: every count below is AS OF THE PRE-WRITE SCAN and
//  EXCLUDES this wave's own forms/fTestCategory.{h,cpp}.  Re-running afterwards
//  returns higher numbers purely because these banners mention the names in
//  prose -- a comment is not a port.  ⚠ AND SEE THE SELF-FALSIFICATION NOTE at
//  the end of the GATE REGISTER: this file's own `#if 0` transcript introduces
//  `Width` and `Height` into wave_preflight's member lists.
//    * `SetShowCateMode` / `sgArm1DrawCell`                 0 files
//    * `SetTestCateCellINT` / `SetTestCateCellAS` /
//      `SetTestingCateCell` / `InitCateCell` /
//      `ShowTestCategory`                                   1 file each --
//      atester_ProcessCount.cpp, and every hit is either banner prose or one
//      of that file's own `PCW7_TESTCAT_*` GATE MACROS (:1705-1711).  A
//      `#define X ((void)(a),(void)(b))` is archive-extraction shape (d), a
//      TU-LOCAL SEAM -- it is not a port of the body and it does not call
//      `fTestCategory->` anything, so it neither satisfies nor is satisfied by
//      this facade.
//    * `AdjFormData`                                        2 files --
//      Command.cpp:9180 / :15785 (both inside `#if 0`, the pre-committed
//      sites) and Automation/auto9045.cpp:893, which calls
//      `W5FA_FTestCategory.AdjFormData()` on a TU-LOCAL struct
//      `W5FA_TfTestCategoryExt` declared at auto9045.cpp:362-366 -- shape (e),
//      a TU-local stand-in, not a port of this class.
//    * `GetTestResult`  ⚠ NAME-SIMILARITY HAZARD, resolved: 7 files.  The
//      hits are a DIFFERENT function -- `int GetTestResult(AnsiString *Data)`,
//      declared Automation/auto9045.h:63 and defined auto9045.cpp:1775, which
//      exists under that exact name in GOLDEN TOO (golden
//      Automation/auto9045.cpp:1775, verified by direct grep of the golden
//      tree 20260828).  Golden's member is
//      `int TfTestCategory::GetTestResult(int,int,int,TColor*)` -- different
//      class, different arity, different parameter types.  Plus
//      SECSGEM/SecsEventType.h:68, an ENUMERATOR.  atester_ProcessCount.cpp:
//      1473-1476 had already reached the same conclusion independently, which
//      is why this facade re-derives rather than trusts it.
//  Renaming shapes probed: `rg "\b(TestCategory_|TestCate_|CateCell)\w+"`
//  -> 0 hits.
//  Conclusion: nothing to re-use, nothing re-translated twice.
//
//  ============================================================================
//  PRE-COMMITTED `#if 0` CALL SITES ELSEWHERE (2 names, 3 sites)
//  ============================================================================
//    Active        PowerSavingMode.cpp:824, spelled `fTestCategory->Active`
//                  inside a `bRe = (fBinSel->Active || ... )` OR chain.
//                  -> matches this facade's `bool Active;` (DEVIATION D-5).
//    AdjFormData   Command.cpp:9180, Command.cpp:15785, both spelled
//                  `fTestCategory->AdjFormData();` -- no arguments, void.
//                  -> matches the declaration below.
//  ⚠ golden's own flag on this class is `bShow`, NOT `fShow` (golden
//  cTestCategory.h:33).  Command.cpp's big "is any form open" chain does NOT
//  list fTestCategory at all, so there is nothing to spell wrongly here -- but
//  do not "normalise" `bShow` to `fShow` on the strength of the sibling
//  facades; the same chain deliberately carries `fCCLink->bShow` with a b.
//  All three sites stay `#if 0` after this wave.
//
//  ============================================================================
//  GATE REGISTER (8)
//  ============================================================================
//  (T-1) FormShow(TObject*)  golden :35-40 (6L) -- TRANSITIVE ONLY.
//        Its three statements are `SetShowCateMode(); AdjFormData(); bShow=
//        true;`.  SetShowCateMode is ACTIVE and bShow is a real field, but
//        AdjFormData is GATE (T-2).  NOT SPLIT: dropping the AdjFormData call
//        would leave the grids un-resized while claiming the form was shown,
//        which is an invented behaviour rather than a translation
//        (忠實優先於「寫得更好」).  Un-gate this one the moment (T-2) opens.
//  (T-2) AdjFormData()  golden :42-143 (102L) -- LINK BOUNDARY + MISSING
//        WIDGET MEMBERS.  RESERVED NAME, so it is DECLARED.
//        (a) LINK: golden :54 calls `IsNNMode()`.  Measured 20260828, the only
//            three definitions in the tree are
//              atester_shims.cpp:340  `int IsNNMode() { return 0; }` -- a STUB
//                  (archive-extraction shape 2), and its file is in
//                  **ht9045_sm**;
//              cinitial.cpp:7253      the real body -- also **ht9045_sm**;
//              tests/test_ga1_cprod.cpp:158 -- a TU-local stand-in, shape (e).
//            ht9045_forms does not link ht9045_sm, and IsNNMode is NOT one of
//            the four sanctioned forms->sm exceptions (RecordProcess,
//            ShowMyMessage, ShowErrorMessage, MyDBIProcess).
//        (b) MISSING MEMBERS: golden writes `MyStringGD[i]->RowHeights[0]` and
//            `sgArm1->Height`.  vclcompat::TStringGrid (vclcompat/
//            StringGrid.h:129-270, read in full 20260828) carries RowCount,
//            ColCount, Cells, Visible, Font, DefaultColWidth, FixedRows,
//            FixedCols, Row, ColWidths[] and ClearRow -- and NO RowHeights and
//            NO Height.  Its own banner (:19-20) says so explicitly.  Golden
//            also writes `fTestCategory->Width`, which this facade does not
//            declare (see D-6).
//        Two independent gates, either one sufficient.
//  (T-3) sgArm1DrawCell(...)  golden :145-359 (215L, 41.5% of the file on its
//        own) -- NO CANVAS LAYER EXISTS.  `MyDrawText` is declared inside
//        `#if 0 // TODO(wave-canvas)` at common.h:387-393, and a declaration
//        inside `#if 0` is not a declaration.  TCanvas, TRect, TGridDrawState,
//        TFontStyles and fsBold likewise have no port.  Also inherits (T-2)(a)
//        via IsNNMode at :166/:178/:211/:259.
//  (T-4) SetTestCateCellINT(int,int,int,int,TColor)  golden :361-390 (30L)
//        -- LINK BOUNDARY, IsNNMode at :363/:375.  See (T-2)(a).
//        ⚠ The BODY is otherwise pure array arithmetic on this object's own
//        TestResult/ColorPtr, which ARE declared below -- so this one opens the
//        moment IsNNMode becomes reachable, with no other work.
//  (T-5) SetTestCateCellAS(int,int,int,AnsiString,TColor)  golden :392-421
//        (30L) -- same single cause as (T-4).
//  (T-6) SetTestingCateCell(int,int,int,TColor)  golden :423-446 (24L)
//        -- same single cause as (T-4).
//  (T-7) GetTestResult(int,int,int,TColor*)  golden :465-494 (30L)
//        -- same single cause as (T-4).
//        ⚠ NOT the same function as the free `int GetTestResult(AnsiString*)`
//        at Automation/auto9045.h:63 -- see the CONDITION-TWO section.
//  (T-8) FormDestroy(TObject*)  golden :533-546 (14L) -- LINK BOUNDARY +
//        MISSING FIELDS.
//        (a) LINK: `LogSoftwareOffTime(AnsiString)` is declared cmydef.h:5032,
//            but its ONLY definition in the tree is acarry_shims.cpp:255
//            (a `{}` no-op) and that file is in **ht9045_sm**.  It is not one
//            of the four sanctioned exceptions.
//        (b) MISSING FIELDS: `PtrDC[]` and `pCanvas` are not declared here --
//            they exist in golden solely to hold a Win32 HDC obtained from
//            `GetDC(sgArm1->Handle)` and a TCanvas, neither of which has any
//            meaning without a window (see D-4).
//        (MyDBIProcess at :543 IS reachable -- 2-arg body aHotPlateSubstrate
//        .cpp:1099, one of the four sanctioned exceptions -- so it is NOT a
//        blocker.  Recorded so a future wave does not re-derive it.)
//
//  ---------------------------------------------------------------------------
//  ⚠ SELF-FALSIFICATION NOTE -- READ BEFORE TRUSTING wave_preflight ON THIS
//  FORM.  The closeout re-run of `wave_preflight.py cTestCategory.cpp`
//  (20260828, AFTER this file landed) reports TWO member names that the
//  opening run did not, and BOTH come from THIS FILE, not from anywhere else
//  in the tree:
//    * `Height`  CONDITIONAL, forms/fTestCategory.cpp:114
//                [#ifdef DEBUG_WIN7_FULL_HD] -- that is the un-taken arm of
//                this facade's own ACTIVE SetShowCateMode.  `Height` IS
//                declared (D-6), so nothing is missing.
//    * `Width`   GATED, forms/fTestCategory.cpp:207/:264/:274/... -- those are
//                lines inside this file's own `#if 0` GATE (T-2) transcript.
//                `Width` is deliberately NOT declared (D-6).
//  Neither is an external reservation.  A future wave must NOT read them as
//  "someone else already committed to these names"; the only consequence is
//  that whoever un-gates (T-2) has to add `int Width` at that time.
//  This is the "波次會把自己的 absence-claim 弄假" trap in its purest form: the
//  falsifying evidence is the wave's own transcript.
//  ---------------------------------------------------------------------------
//
//  ============================================================================
//  ACTIVE (4, plus the PARTIAL ctor) -- reachability evidence
//  ============================================================================
//   TfTestCategory()      golden :17-33, ACTIVE-PARTIAL.  Live = golden
//        :26-29 (`EdgeWidth=80; EdgeHeight=24; bCateByArm=false; bShow=false;`)
//        -- four own-field writes, no global deref, no call.  SIOF-safe
//        (docs/KNOWLEDGE.md "static-init ctor 不可碰 NULL 全域"; the
//        fLaserSensor incident that rule comes from turned 88 of 134 ctest
//        binaries into SEGFAULTs).  GATED = golden :20-25 (the
//        `MyStringGD[i]->Tag=i` loop -- vclcompat::TStringGrid derives from
//        TObject, NOT TControl, so it has no Tag), :30-31 (`GetDC(sgArm1->
//        Handle)` -- no window, no Handle) and :32 (`pCanvas=new TCanvas`).
//   InitCateCell()        golden :496-509.  Triple loop over this object's own
//        ColorPtr/TestResult; the only external names are `clWhite`
//        (vclcompat/LedCore.h:61, header-only const) and TColor
//        (cmydef.h:16, `typedef int`).  No IsNNMode -- which is exactly why
//        this one is ACTIVE while its four siblings are not.
//   SetShowCateMode()     golden :511-525.  Reads `IniConfig.iShowCateByArm`
//        (Config.h:731 -> Config.cpp, ht9045_globals) and writes this facade's
//        own `Height` (D-6).  ⚠ `DEBUG_WIN7_FULL_HD` is NOT defined anywhere
//        in this tree (0 hits in CMakeLists.txt / MachineDefine.h /
//        MachineType.h, 20260828), so the `#else` arm (192 / 105) is what
//        compiles.  The `#ifdef` is kept verbatim rather than folded away.
//   ShowTestCategory(int) golden :448-463.  Calls ACTIVE SetShowCateMode,
//        reads `bUseTwoArm32Site` (cmydef.h:4836 -> cmydef.cpp,
//        ht9045_globals) and calls `Refresh()` on the two grids (D-3).
//   FormClose()           golden :527-531.  `bShow=false;`.
//
//  ============================================================================
//  DEVIATIONS (port-only; each with why)
//  ============================================================================
//  (D-1) `__fastcall` dropped from every signature -- tree-wide convention.
//  (D-2) `TfTestCategory` has NO base class (golden: `: public TForm`).
//      Matches forms/fCounterSel.h (D-2) / forms/fBuilder.h (D-2).
//  (D-3) `TfTestCategoryGrid` is a facade-local subclass of
//      vclcompat::TStringGrid adding exactly ONE member, `Refresh()`, as an
//      offline no-op.  ACTIVE ShowTestCategory needs it; vclcompat's grid does
//      not have it; and forms/FormWidgets.h's own banner says a wave should
//      NOT edit that shared header.  Subclassing in the consuming form's own
//      header is precisely how forms/fObserver.h's TfObserverGrid and
//      forms/fConfiguration.h's TfConfigurationGrid started life (both were
//      later collapsed INTO vclcompat by W906-VclGrid-1 -- the same path is
//      open to Refresh() if a second consumer appears).
//      ⚠ `Refresh()` offline is genuinely a no-op: there is no window and no
//      OnDrawCell handler to re-enter.  It is NOT claimed to repaint anything.
//  (D-4) golden's private `MyStringGD[2]`, `PtrDC[2]` and `pCanvas` are NOT
//      declared.  MyStringGD is touched only by the GATED ctor tail and GATED
//      AdjFormData; PtrDC/pCanvas only by the GATED ctor tail, GATED
//      sgArm1DrawCell and GATED FormDestroy (minimal-declaration idiom --
//      forms/fSetup.h:20-25's explicit refusal to grow a facade past measured
//      need).  Their names appear only inside `#if 0`, which never compiles.
//  (D-5) `bool Active` is declared although golden's TfTestCategory has NO
//      such member -- it is inherited `TForm::Active`.  It is a RESERVED NAME
//      (PowerSavingMode.cpp:824) and D-2 removes the base class, so it must be
//      re-declared here or that pre-committed site can never bind.  See
//      ZERO-WRITER for what it will read.
//  (D-6) `int Height = 0;` is declared, value-only, no layout (the same
//      "只存值" contract as vclcompat/Controls.h:271-272 and
//      forms/fLd_ULd.h:348-349).  Written by ACTIVE SetShowCateMode.
//      `Width` is deliberately NOT declared: golden writes it only inside
//      GATED AdjFormData (:44, :101, :111-112, :125).
//  (D-7) `FormClose(TObject*, TCloseAction&)` -> `void FormClose();`.
//      TCloseAction has no port and golden's body reads neither parameter.
//      Precedent forms/fCounterSel.h (D-4) / forms/fBuilder.h (D-4).
//  (D-8) `sgArm1DrawCell(TObject*, int, int, TRect&, TGridDrawState)` is
//      declared as `sgArm1DrawCell(TObject *Sender, int ACol, int ARow)`.
//      TRect and TGridDrawState have ZERO port in this tree (20260828), and
//      the body's only use of `Rect` is as an argument to the un-ported
//      MyDrawText, while `State` is never read at all.  The body is GATED
//      either way; the reduced signature exists so the declaration itself
//      compiles.  Same reduction shape as D-6 on forms/fBuilder.h.
//
//  ============================================================================
//  FIELD LIST (2 widgets + 2 arrays + 5 scalars)
//  ============================================================================
//    bShow      (bool)                 golden cTestCategory.h:33
//    Active     (bool)                 DEVIATION D-5, PRE-COMMITTED (1 site)
//    Height     (int)                  DEVIATION D-6, ACTIVE SetShowCateMode
//    EdgeWidth  (int)                  golden cTestCategory.h:20, ACTIVE ctor
//    EdgeHeight (int)                  golden cTestCategory.h:21, ACTIVE ctor
//    bCateByArm (bool)                 golden cTestCategory.h:29, ACTIVE ctor
//    TestResult[2][4][8] (int)         golden cTestCategory.h:26, ACTIVE InitCateCell
//    ColorPtr  [2][4][8] (TColor)      golden cTestCategory.h:27, ACTIVE InitCateCell
//    sgArm1 / sgArm2 (TfTestCategoryGrid*)  golden cTestCategory.h:11-12
//  NOT declared: see DEVIATION (D-4).
//  ⚠ golden's array bounds are [2][4][8] and its own commented-out predecessor
//  (:23-24) was [2][4][4].  The [2][4][8] shape is kept exactly -- InitCateCell
//  loops i<2, j<4, k<8 and must stay in range.
//
//  ============================================================================
//  ZERO-WRITER FIELDS (task rule: state the consequence)
//  ============================================================================
//  * `Active` has NO writer anywhere -- in golden it is maintained by the VCL
//    form machinery, which D-2 removes.  CONSEQUENCE: the moment
//    PowerSavingMode.cpp:824 is un-gated, `fTestCategory->Active` answers
//    "this form is never focused".  In that OR chain that biases the power-save
//    decision toward "no form active".  Flagged per task rule; it is a
//    consequence of not porting TForm, not a defect in this facade.
//  * `TestResult` / `ColorPtr` have exactly ONE writer in this port, ACTIVE
//    InitCateCell (clWhite / -1).  Their four real producers are GATE
//    (T-4)..(T-6) and their only consumer is GATE (T-7).  That is
//    self-consistent -- and it is the SAME resting state
//    atester_ProcessCount.cpp's PCW7_TESTCAT_GETRESULT macro (:1711) already
//    hard-codes (`*(pColor)=clWhite, -1`), derived there from golden
//    InitCateCell:504-505.  The two agree, independently.
//  * `bShow` is written false by the ACTIVE ctor and by ACTIVE FormClose, and
//    true only by GATED FormShow -- so it is permanently false today.  It has
//    no reader in this port (`rg "\bbShow\b" ...` hits are other classes'
//    members: fCCLink, fBinSel, fBarCode, fLtcSensor, fOmron, fFTPClient).
//
//  ============================================================================
//  DISCLOSURE (task rule 8 -- what was NOT read to completion / NOT verified)
//  ============================================================================
//  - vclcompat/StringGrid.h was read in FULL (the whole class, :129-270),
//    because the (T-2)(b) gate rests on the ABSENCE of RowHeights/Height.
//    vclcompat/Controls.h was read only in the region :180-510; the claim
//    "TStringGrid has no Tag" rests on TStringGrid deriving from TObject
//    (StringGrid.h:129) rather than TControl, which is where Tag lives
//    (Controls.h:255).
//  - `IsNNMode`'s three definitions were located and their owning
//    `add_library` blocks confirmed line-accurately, but the BODY of
//    cinitial.cpp:7253 was NOT read.  Immaterial: the gate is the archive
//    edge, not the semantics.
//  - The mode enumerators golden AdjFormData switches on (SingleSite,
//    TriSite1X3, _6Site2X3N, _8Site2X4N, _10Site2X5, _12Site2X6, _16Site2X8,
//    _32Site4X8N, _32Site4X8M, _16Site4X4, DualSite2x1, QualSite1X4,
//    _8Site1X4, _8Site2X4) were NOT individually confirmed to exist in this
//    port.  All 14 appear only inside `#if 0`.
//  - golden `cTestCategory.dfm` was NOT opened.  No ACTIVE body needs
//    design-time geometry -- but note that both grids here default to
//    vclcompat's 5x5, not to golden's .dfm dimensions, so a wave that un-gates
//    AdjFormData or sgArm1DrawCell must supply the real ones.
// =============================================================================
#ifndef FORMS_FTESTCATEGORY_H
#define FORMS_FTESTCATEGORY_H

#include "forms/FormWidgets.h"   // AnsiString / TObject / TStringGrid (vclcompat)
#include "cmydef.h"              // typedef int TColor (cmydef.h:16)

// ===========================================================================
//  DEVIATION D-3 -- facade-local subclass of vclcompat::TStringGrid adding the
//  single member ACTIVE ShowTestCategory needs.  ⚠ Offline no-op: there is no
//  window and no OnDrawCell handler, so this repaints nothing.
// ===========================================================================
class TfTestCategoryGrid : public vclcompat::TStringGrid
{
public:
    virtual ~TfTestCategoryGrid() {}
    void Refresh() {}      // golden TStringGrid::Refresh -- offline no-op
};

// ===========================================================================
//  TfTestCategory -- non-VCL facade (golden cTestCategory.h:8-42,
//  `class TfTestCategory : public TForm`; NO base class here, DEVIATION D-2).
// ===========================================================================
class TfTestCategory
{
public:
    TfTestCategory();

    // -- golden public: User declarations ------------------------------------
    bool bShow = false;                                 // golden cTestCategory.h:33 (NOT `fShow` -- see banner)

    // -- DEVIATION D-5 / D-6: golden's inherited TForm members, value-only ----
    bool Active = false;                                // DEVIATION D-5, PRE-COMMITTED (PowerSavingMode.cpp:824)
    int  Height = 0;                                    // DEVIATION D-6, ACTIVE SetShowCateMode writes 192 / 105

    // -- golden private: User declarations (only what an ACTIVE body needs) --
    int    EdgeWidth  = 0;                              // golden cTestCategory.h:20, ACTIVE ctor writes 80
    int    EdgeHeight = 0;                              // golden cTestCategory.h:21, ACTIVE ctor writes 24
    bool   bCateByArm = false;                          // golden cTestCategory.h:29
    int    TestResult[2][4][8] = {};                    // golden cTestCategory.h:26  (jou 981226 : for 9046)
    TColor ColorPtr  [2][4][8] = {};                    // golden cTestCategory.h:27

    // -- golden __published widgets touched by an ACTIVE body ----------------
    TfTestCategoryGrid *sgArm1 = new TfTestCategoryGrid();   // golden cTestCategory.h:12
    TfTestCategoryGrid *sgArm2 = new TfTestCategoryGrid();   // golden cTestCategory.h:11

    // -- ACTIVE this wave (defined in forms/fTestCategory.cpp) ---------------
    void InitCateCell();                                // golden :496-509  ACTIVE
    void SetShowCateMode();                             // golden :511-525  ACTIVE
    void ShowTestCategory(int iIndex);                  // golden :448-463  ACTIVE
    void FormClose();                                   // golden :527-531  ACTIVE, DEVIATION D-7

    // -- GATED this wave -- declared here, golden body translated in
    //    forms/fTestCategory.cpp inside `#if 0`.  Calling one is a LINK ERROR
    //    by design (see GATE REGISTER above). ---------------------------------
    void FormShow(TObject *Sender);                                       // golden :35-40    GATE (T-1)
    void AdjFormData();                                                   // golden :42-143   GATE (T-2)  RESERVED NAME
    void sgArm1DrawCell(TObject *Sender, int ACol, int ARow);             // golden :145-359  GATE (T-3), DEVIATION D-8
    void SetTestCateCellINT(int Arm, int X, int Y, int Bin, TColor Color); // golden :361-390 GATE (T-4)
    void SetTestCateCellAS(int Arm, int X, int Y, AnsiString Bin, TColor Color); // golden :392-421 GATE (T-5)
    void SetTestingCateCell(int Arm, int X, int Y, TColor Color);         // golden :423-446  GATE (T-6)
    int  GetTestResult(int Arm, int Row, int Col, TColor *CellColor);     // golden :465-494  GATE (T-7)
    void FormDestroy(TObject *Sender);                                    // golden :533-546  GATE (T-8)
};

// AI(W906-FW3-BTQ1) 20260828: TfTestCategory/fTestCategory were FREE tree-wide
// (see the banner's measured provenance) -- declared here so the 3
// pre-committed `#if 0` sites (PowerSavingMode.cpp:824, Command.cpp:9180,
// Command.cpp:15785) have something real to bind to once THEIR own gates are
// separately lifted (NOT done by this wave).
extern TfTestCategory *fTestCategory;

#endif // FORMS_FTESTCATEGORY_H
