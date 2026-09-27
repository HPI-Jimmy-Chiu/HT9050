// =============================================================================
//  tests/test_w7_f0_controls_guard.cpp
//
//  AI(W906-W7-F0) 20260728: the W7-F0-b OVERLOAD GUARD, plus the default-value
//  regression that makes W7-F0's "zero behaviour change" claim testable instead
//  of merely asserted.  See docs/W7_UI_ARCHITECTURE_PLAN.md SS6-F0-b / SS4-V2 /
//  SS9-R8.
//
//  ============================ PART A: WHY ==================================
//  SECSGEM/SecsSvEcRegistration.h declares SAME-ARITY overload families:
//
//    SetSVDataPointer(..., void       *P, AnsiString Remark)            :133
//    SetSVDataPointer(..., AnsiString *P, AnsiString Remark)            :139
//    SetSVDataPointer(..., void       *P, int Len, AnsiString Remark)   :144
//    SetSVDataPointer(..., TObject    *P, AnsiString Remark)            :149
//
//    SetECDataPointer(..., AnsiString *P, AnsiString,AnsiString,AnsiString,
//                                         AnsiString)                   :154
//    SetECDataPointer(..., void       *P, int,int,int, AnsiString)       :162
//    SetECDataPointer(..., TObject    *P, const char*,const char*,
//                                         const char*, AnsiString)      :180
//    SetECDataPointer(..., void       *P, AnsiString,AnsiString,
//                                         AnsiString, AnsiString)       :187
//
//  A widget stand-in with NO `TObject` base can only reach `void *P`, so it
//  binds the RAW-SCALAR overload -- silently, with no diagnostic, registering a
//  widget as if it were a pointer to one scalar value.  With a TObject base,
//  [over.ics.rank]/4.4.2 ("conversion of B* to A* is better than conversion of
//  B* to void*") makes the TObject* overload strictly better, so it wins.
//
//  ~1740 real SV/EC registrations (uHGemHT9045_SV.cpp / _EC.cpp) are still to
//  land.  If even one widget type loses its TObject base, those registrations
//  start mis-binding somewhere in the middle of a 1740-site wave with nothing
//  to show it.  This file makes that a COMPILE ERROR instead.
//
//  ========================= PART A: HOW IT IS PROVED ========================
//  The overloads return `void`, so which one a call selects cannot be observed
//  directly.  The standard technique is a MIRROR: probe functions with the exact
//  same parameter lists but distinct class return types, so `sizeof` of the call
//  expression (an unevaluated operand -- nothing is constructed or called)
//  reports which overload won.  The mirrors below are transcribed
//  parameter-for-parameter from the real declarations above.
//
//  A guard that cannot fail proves nothing, so PART A also carries NEGATIVE
//  CONTROLS: `W7F0_NoBase` (a struct with no base) and `int` must both bind the
//  `void*` overload.  If a future change accidentally made everything look like
//  a TObject, those static_asserts fire.
//
//  ============= PART B: THE DEFAULT-VALUE REGRESSION (zero-change) ==========
//  W7-F0 replaced ~11 bespoke `struct Tf...` value holders with typedefs onto
//  vclcompat/Controls.h.  A typedef only preserves BEHAVIOUR if the constructed
//  state is identical, so PART B pins the retired structs' exact initial values.
//  Each case cites the retired definition verbatim.
//
//  ================ FORMER KNOWN GAP, NOW CLOSED (MEDIUM-2 fix) ==============
//  vclcompat::TStringGrid (vclcompat/StringGrid.h) -- the `Cells` provider,
//  446 measured golden uses, and a genuine stock widget (golden main.h:457
//  `TStringGrid *AutoCleanStringGrid;`) -- used to be declared
//  `class TStringGrid {` with NO base, which would have made a future
//  `SetSVDataPointer(..., someStringGrid, ...)` silently bind the `void*`
//  overload: the exact SS9-R8 hazard.  AI(W906-F0fix) 20260728: gave it
//  `: public vclcompat::TObject` (verified safe first -- every construction
//  site in the tree is `new TStringGrid(...)` through a pointer, no by-value
//  or aggregate-init use exists; see StringGrid.h's own note) and moved it
//  into the positive assertion list below, next to the other stock types.
// =============================================================================
#include "forms/FormWidgets.h"   // the facade's widget layer -> vclcompat/Controls.h

#include <cstdio>

static int g_pass = 0;
static int g_fail = 0;

static void check(const char *name, bool ok)
{
    if (ok) { printf("PASS  %s\n", name); ++g_pass; }
    else    { printf("FAIL  %s\n", name); ++g_fail; }
}

// =============================================================================
//  PART A -- compile-time overload guard
// =============================================================================
namespace w7f0_guard {

// Distinct sizes so `sizeof` identifies the winner unambiguously.
struct BindVoid    { char pad[1]; };
struct BindAnsiPtr { char pad[2]; };
struct BindTObject { char pad[3]; };
struct BindVoidLen { char pad[4]; };

// --- mirror of SetSVDataPointer's 4 overloads (SecsSvEcRegistration.h:133-150)
BindVoid    sv_probe(AnsiString, unsigned char, AnsiString, AnsiString, void *,       AnsiString);
BindAnsiPtr sv_probe(AnsiString, unsigned char, AnsiString, AnsiString, AnsiString *, AnsiString);
BindVoidLen sv_probe(AnsiString, unsigned char, AnsiString, AnsiString, void *, int,  AnsiString);
BindTObject sv_probe(AnsiString, unsigned char, AnsiString, AnsiString, vclcompat::TObject *, AnsiString);

// --- mirror of SetECDataPointer's 4 overloads (SecsSvEcRegistration.h:154-190)
BindAnsiPtr ec_probe(AnsiString, unsigned char, AnsiString, AnsiString, AnsiString *,
                     AnsiString, AnsiString, AnsiString, AnsiString);
BindVoidLen ec_probe(AnsiString, unsigned char, AnsiString, AnsiString, void *,
                     int, int, int, AnsiString);
BindTObject ec_probe(AnsiString, unsigned char, AnsiString, AnsiString, vclcompat::TObject *,
                     const char *, const char *, const char *, AnsiString);
BindVoid    ec_probe(AnsiString, unsigned char, AnsiString, AnsiString, void *,
                     AnsiString, AnsiString, AnsiString, AnsiString);

// Negative control: no base at all -- must reach only the void* overloads.
struct W7F0_NoBase { AnsiString Text; };

} // namespace w7f0_guard

// sizeof's operand is unevaluated: no AnsiString is constructed, no probe is
// called, and none of these probes is ever defined (deliberately -- if one were
// ever ODR-used the link would fail loudly rather than run something).
#define W7F0_SV_PICK(TYPE)                                                     \
    sizeof(w7f0_guard::sv_probe(AnsiString(""), (unsigned char)0,              \
                                AnsiString(""), AnsiString(""),                \
                                (TYPE *)0, AnsiString("")))

#define W7F0_EC_PICK(TYPE)                                                     \
    sizeof(w7f0_guard::ec_probe(AnsiString(""), (unsigned char)0,              \
                                AnsiString(""), AnsiString(""),                \
                                (TYPE *)0, "", "", "", AnsiString("")))

#define W7F0_ASSERT_BINDS_TOBJECT(TYPE)                                        \
    static_assert(W7F0_SV_PICK(TYPE) == sizeof(w7f0_guard::BindTObject),        \
        #TYPE " must bind SetSVDataPointer's TObject* overload "               \
        "(SecsSvEcRegistration.h:149), not the void* one (:133). "             \
        "Give it a `: public vclcompat::TObject` base -- see "                 \
        "vclcompat/Controls.h and plan SS4-V2/SS9-R8.");                       \
    static_assert(W7F0_EC_PICK(TYPE) == sizeof(w7f0_guard::BindTObject),        \
        #TYPE " must bind SetECDataPointer's TObject* overload "               \
        "(SecsSvEcRegistration.h:180), not a void* one (:162/:187).")

#define W7F0_ASSERT_BINDS_VOID(TYPE)                                           \
    static_assert(W7F0_SV_PICK(TYPE) == sizeof(w7f0_guard::BindVoid),           \
        "NEGATIVE CONTROL BROKEN: " #TYPE " has no TObject base yet did not "  \
        "bind the void* SV overload -- the guard above is now vacuous.");       \
    static_assert(W7F0_EC_PICK(TYPE) == sizeof(w7f0_guard::BindVoid),           \
        "NEGATIVE CONTROL BROKEN: " #TYPE " did not bind the void* EC overload.")

// --- the unified stock types (vclcompat/Controls.h) -------------------------
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TFont);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TControl);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TLabel);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TPanel);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TGroupBox);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TCustomEdit);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TEdit);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TLabeledEdit);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TMemo);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TComboBox);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TListBox);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TCheckBox);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TRadioButton);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TRadioGroup);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TButton);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TBitBtn);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TSpeedButton);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TPageControl);
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TTabSheet);
// TStringList is the 7th SECS cast target and got its base in W906-VCW1's R1.
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TStringList);
// TStringGrid: MEDIUM-2 fix, AI(W906-F0fix) 20260728 -- see file-head note above.
W7F0_ASSERT_BINDS_TOBJECT(vclcompat::TStringGrid);

// --- the facade's own aliases (forms/FormWidgets.h PART 1) ------------------
//     Same types by definition, asserted through their FACADE spelling so a
//     future re-alias onto something base-less is caught at its own name.
W7F0_ASSERT_BINDS_TOBJECT(TfMainCheckBox);
W7F0_ASSERT_BINDS_TOBJECT(TfMainFont);
W7F0_ASSERT_BINDS_TOBJECT(TfMainPageControl);
W7F0_ASSERT_BINDS_TOBJECT(TfMainPanel);
W7F0_ASSERT_BINDS_TOBJECT(TfMainTrayPanel);
W7F0_ASSERT_BINDS_TOBJECT(TfSortCTPanel);
W7F0_ASSERT_BINDS_TOBJECT(TfLotInfoRunMode);
W7F0_ASSERT_BINDS_TOBJECT(TfLotInfoLabel);
W7F0_ASSERT_BINDS_TOBJECT(TfLotInfoStatusLabel);
W7F0_ASSERT_BINDS_TOBJECT(TfLotInfoPanel);
W7F0_ASSERT_BINDS_TOBJECT(TfLotInfoEdit);

// --- the DEFERRED bespoke stand-ins (forms/FormWidgets.h PART 2) ------------
//     Not aliased, but they DO carry the TObject base, which is the half of
//     F0-a that actually prevents the mis-bind.  That is what these assert.
W7F0_ASSERT_BINDS_TOBJECT(TfMainSiteMapLog);
W7F0_ASSERT_BINDS_TOBJECT(TfMainGrid);
W7F0_ASSERT_BINDS_TOBJECT(TfMainAutoCleanGrid);
W7F0_ASSERT_BINDS_TOBJECT(TfLedValue);
W7F0_ASSERT_BINDS_TOBJECT(TfMainSpeedButton);
W7F0_ASSERT_BINDS_TOBJECT(TfMainMemo);
W7F0_ASSERT_BINDS_TOBJECT(TfMainMemoLines);

// --- negative controls ------------------------------------------------------
W7F0_ASSERT_BINDS_VOID(w7f0_guard::W7F0_NoBase);
W7F0_ASSERT_BINDS_VOID(int);

int main()
{
    printf("=== W7-F0 controls guard ===\n");
    printf("NOTE  PART A (overload binding: 39 W7F0_ASSERT_BINDS_TOBJECT sites +\n");
    printf("NOTE  2 negative controls, 2 static_asserts each) is a\n");
    printf("NOTE  COMPILE-TIME guard: this executable existing at all means every\n");
    printf("NOTE  static_assert in it held.  See this file's head for the mirror\n");
    printf("NOTE  technique (vclcompat::TStringGrid's former gap is now closed and\n");
    printf("NOTE  covered by its own assertion above, MEDIUM-2 fix).\n");

    // -----------------------------------------------------------------------
    //  PART A(runtime) -- the polymorphism the guard relies on is real, and the
    //  SECS cast cascade still discriminates between sibling widget types.
    //  (dynamic_cast is only well-formed at all because the TObject base is
    //  there, so these lines are themselves a compile-time check too.)
    // -----------------------------------------------------------------------
    {
        vclcompat::TPanel      panel;
        vclcompat::TCustomEdit edit;
        vclcompat::TEdit       derivedEdit;
        vclcompat::TObject    *asObj = &panel;

        check("dynamic_cast<TPanel*> of a TPanel-as-TObject* matches",
              dynamic_cast<vclcompat::TPanel *>(asObj) != 0);
        check("dynamic_cast<TLabel*> of a TPanel-as-TObject* does NOT match (siblings stay distinct)",
              dynamic_cast<vclcompat::TLabel *>(asObj) == 0);
        check("dynamic_cast<TCustomEdit*> of a TPanel-as-TObject* does NOT match",
              dynamic_cast<vclcompat::TCustomEdit *>(asObj) == 0);

        asObj = &edit;
        check("dynamic_cast<TCustomEdit*> of a TCustomEdit-as-TObject* matches",
              dynamic_cast<vclcompat::TCustomEdit *>(asObj) != 0);
        check("dynamic_cast<TPanel*> of a TCustomEdit-as-TObject* does NOT match",
              dynamic_cast<vclcompat::TPanel *>(asObj) == 0);

        // golden's cascade casts to TCustomEdit, so a TEdit/TMemo/TLabeledEdit
        // must ALSO match it -- that is why the edit family is a hierarchy.
        asObj = &derivedEdit;
        check("dynamic_cast<TCustomEdit*> of a TEdit-as-TObject* matches (golden casts to the base)",
              dynamic_cast<vclcompat::TCustomEdit *>(asObj) != 0);
    }

    // -----------------------------------------------------------------------
    //  PART B -- retired bespoke structs' exact initial state, pinned.
    //  Each retired definition is quoted from the pre-W7-F0 FormsFacade.h.
    // -----------------------------------------------------------------------
    {
        // `struct TfMainCheckBox { bool Checked; TfMainCheckBox():Checked(false){} };`
        TfMainCheckBox cb;
        check("TfMainCheckBox default Checked==false (retired struct's exact default)",
              cb.Checked == false);

        // `struct TfMainFont { int Color; TfMainFont():Color(0){} };`
        TfMainFont fnt;
        check("TfMainFont default Color==0", fnt.Color == 0);

        // `struct TfMainPageControl { int ActivePageIndex; ...:ActivePageIndex(0){} };`
        TfMainPageControl pc;
        check("TfMainPageControl default ActivePageIndex==0", pc.ActivePageIndex == 0);

        // `struct TfMainPanel { AnsiString Caption; };`  (AnsiString default "")
        TfMainPanel mp;
        check("TfMainPanel default Caption==\"\"", mp.Caption == AnsiString(""));

        // `struct TfMainTrayPanel { int Color; bool Visible;
        //   TfMainTrayPanel():Color(0),Visible(false){} };`
        TfMainTrayPanel tp;
        check("TfMainTrayPanel default Color==0", tp.Color == 0);
        check("TfMainTrayPanel default Visible==false", tp.Visible == false);

        // `struct TfSortCTPanel { AnsiString Caption; };`
        TfSortCTPanel sp;
        check("TfSortCTPanel default Caption==\"\"", sp.Caption == AnsiString(""));

        // `struct TfLotInfoRunMode { bool Visible; AnsiString Text;
        //   TfLotInfoRunMode():Visible(false){} };`
        // The Visible==false half is LOAD-BEARING: ainarm_SearchPlacePlate.cpp:4811
        // reads `fLotInfo->cbRunMode->Visible==true` on a live branch, so a
        // VCL-faithful `true` default here would silently change behaviour.
        TfLotInfoRunMode rm;
        check("TfLotInfoRunMode default Visible==false (LOAD-BEARING, read at ainarm_SearchPlacePlate.cpp:4811)",
              rm.Visible == false);
        check("TfLotInfoRunMode default Text==\"\"", rm.Text == AnsiString(""));

        // `struct TfLotInfoLabel { AnsiString Caption; };`
        TfLotInfoLabel ll;
        check("TfLotInfoLabel default Caption==\"\"", ll.Caption == AnsiString(""));

        // `struct TfLotInfoStatusLabel { AnsiString Caption; int Color;
        //   TfLotInfoStatusLabel():Color(0){} };`
        TfLotInfoStatusLabel sl;
        check("TfLotInfoStatusLabel default Caption==\"\"", sl.Caption == AnsiString(""));
        check("TfLotInfoStatusLabel default Color==0", sl.Color == 0);

        // `struct TfLotInfoEdit { AnsiString Text; };`
        TfLotInfoEdit le;
        check("TfLotInfoEdit default Text==\"\"", le.Text == AnsiString(""));

        // `struct TfLotInfoPanel { bool Enabled; bool Visible;
        //   TfLotInfoPanel():Enabled(true),Visible(true){} };`
        // The unified TPanel defaults BOTH to false, so the true/true state is
        // restored explicitly in forms/fLotInfo.cpp's ctor.  This pair of checks
        // pins the TYPE's default (false) -- the restored instance state is what
        // tests/test_agv_portscan.cpp exercises.
        TfLotInfoPanel lp;
        check("TfLotInfoPanel (unified TPanel) type default Enabled==false -- fLotInfo ctor restores true",
              lp.Enabled == false);
        check("TfLotInfoPanel (unified TPanel) type default Visible==false -- fLotInfo ctor restores true",
              lp.Visible == false);

        // Deferred stand-ins keep their own retired defaults verbatim.
        TfLedValue led;
        check("TfLedValue default Value==false", led.Value == false);
        TfMainSpeedButton sb;
        check("TfMainSpeedButton default Down==false", sb.Down == false);
        TfMainAutoCleanGrid acg;
        check("TfMainAutoCleanGrid defaults XItem/YItem/Top/Width/Height all 0",
              acg.XItem == 0 && acg.YItem == 0 && acg.Top == 0 &&
              acg.Width == 0 && acg.Height == 0);
        TfMainMemo mm;
        check("TfMainMemo ctor allocates Lines", mm.Lines != 0);
        check("TfMainMemo Lines->Count==0 and Add() is still a no-op sink (deferred, see FormWidgets.h)",
              mm.Lines != 0 && mm.Lines->Count == 0);
        if (mm.Lines) { mm.Lines->Add(AnsiString("x")); }
        check("TfMainMemo Lines->Count STILL 0 after Add() -- the no-op semantics W7-F0 deliberately did NOT change",
              mm.Lines != 0 && mm.Lines->Count == 0);
    }

    // -----------------------------------------------------------------------
    //  PART C -- the new unified types' own defaults, so a later edit to
    //  Controls.h cannot quietly move them.
    // -----------------------------------------------------------------------
    {
        vclcompat::TControl c;
        check("TControl default Visible==false / Enabled==false / hCtl==NULL",
              c.Visible == false && c.Enabled == false && c.hCtl == 0);

        vclcompat::TMemo memo;
        check("vclcompat::TMemo allocates a REAL TStringList for Lines",
              memo.Lines != 0);
        memo.Lines->Add(AnsiString("a"));
        memo.Lines->Add(AnsiString("b"));
        check("vclcompat::TMemo::Lines has REAL Add/Count semantics (Count==2)",
              memo.Lines->Count == 2);
        memo.Clear();
        check("vclcompat::TMemo::Clear() forwards to Lines->Clear() (golden TMemo::Clear)",
              memo.Lines->Count == 0);

        vclcompat::TComboBox combo;
        check("vclcompat::TComboBox allocates a REAL TStringList for Items",
              combo.Items != 0);
        check("vclcompat::TComboBox default ItemIndex==0", combo.ItemIndex == 0);

        // AI(W906-FW3-Observer-W2) 20260825: RECALIBRATED. This used to assert
        // `rg.Items == 0` -- an expectation calibrated to the W7-F0 refactor's
        // own zero-change contract, not to golden. Real VCL's TRadioGroup
        // allocates ->Items in its constructor, and cObserver's FormShow
        // (golden :444-452, :489-497) dereferences it on three TRadioGroups, so
        // the NULL default was a crash waiting for the first consumer rather
        // than a property worth pinning. vclcompat::TRadioGroup now matches
        // TComboBox/TListBox; this guard pins the NEW invariant.
        vclcompat::TRadioGroup rg;
        check("vclcompat::TRadioGroup allocates a REAL TStringList for Items",
              rg.Items != 0);
        check("vclcompat::TRadioGroup default ItemIndex==0", rg.ItemIndex == 0);
        rg.Items->Add(AnsiString("Row-A"));
        rg.Items->Add(AnsiString("Row-B"));
        check("vclcompat::TRadioGroup::Items has REAL Add/Count semantics (Count==2)",
              rg.Items->Count == 2);

        vclcompat::TCustomEdit ce;
        ce.Text = AnsiString("something");
        ce.Clear();
        check("vclcompat::TCustomEdit::Clear() empties Text", ce.Text == AnsiString(""));
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
