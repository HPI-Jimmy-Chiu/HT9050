// =============================================================================
//  forms/fShuttleMove.h  --  non-VCL stand-in for golden's fShuttleMove form ptr
//
//  AI(W906-W7-L2) 20260803: NEW FILE.  First home for fShuttleMove anywhere in
//  this migrated tree (golden ShuttleMove.h:14 `class TfShuttleMove : public
//  TForm`; global at golden ShuttleMove.h:121 `extern PACKAGE TfShuttleMove
//  *fShuttleMove;`, defined golden ShuttleMove.cpp:38).  Verified absent before
//  writing: the only `fShuttleMove` / `TfShuttleMove` tokens anywhere in the
//  ported tree were rows of the dfm2rc-generated layout table under build/
//  (build/dfm2rc_regen_fidelity/.../ShuttleMove_layout.gen.cpp) -- generated
//  IR data, no class and no global -- so there is no collision.
//
//  WHY IT IS NEEDED (W7-L2 substrate for golden ckernel.cpp)
//  ---------------------------------------------------------
//  golden ckernel.cpp's WaitManualRetryKey (:133-149, //JerryYang 20160728)
//  dereferences fShuttleMove at EXACTLY 4 sites and nowhere else in the whole
//  2589-line file (measured by reading ckernel.cpp end-to-end this wave):
//      :138  fShuttleMove->btRetry->Caption="Retry";
//      :140  fShuttleMove->btRetry->Caption="";
//      :142  if(fShuttleMove->bShuttleRetry || Sen[SnFKRetry].IsOn() || ...)
//      :145      fShuttleMove->bShuttleRetry=false;
//  Those 4 are also, tree-wide in GOLDEN, the ONLY uses of either member
//  outside ShuttleMove.cpp/.h itself (grep over the whole golden tree this
//  wave: 4 hits, all in ckernel.cpp).  So this facade's surface is closed --
//  no later golden caller can widen it.
//
//  MINIMAL SURFACE, DELIBERATELY
//  -----------------------------
//  golden's TfShuttleMove is a 2762-line VCL maintenance form (ShuttleMove.cpp)
//  with ~50 published widgets and 15 public methods (DoShuttleMove,
//  DoScanDeviceOnShuttle1, VerifySafetyPositionInOutIndex, ...).  None of that
//  is in scope: this is the "form as junk drawer" facade idiom already used by
//  TfNote / TfCleaning / TfOCR in this directory.  Two members are landed, both
//  because ckernel.cpp:133-149 touches them.  Notably NOT landed:
//    * `bool fShow` (golden ShuttleMove.h:96) -- real, and set false by golden's
//      ctor at ShuttleMove.cpp:62, but ckernel.cpp never reads it.  Adding it
//      would be inventing an offline default (i.e. a silent branch selection)
//      for a branch that does not exist in this wave's scope.
//    * `iShuttleMoveTask` / `bSetupStep` / the Verify*SafetyPosition methods --
//      same reason.  A later wave that translates ShuttleMove.cpp itself adds
//      them, with its own justification, at that time.
//
//  Facade-wide contract: see forms/fMain.h.
// =============================================================================
#ifndef FORMS_FSHUTTLEMOVE_H
#define FORMS_FSHUTTLEMOVE_H

#include "forms/FormWidgets.h"

// ===========================================================================
//  TfShuttleMove -- non-VCL stand-in (golden ShuttleMove.h:14)
// ===========================================================================
class TfShuttleMove
{
public:
    // [DATA] golden ShuttleMove.h:110 `bool bShuttleRetry;`
    //
    // OFFLINE DEFAULT false -- THIS IS A BRANCH SELECTION, stated explicitly:
    // false OPENS the "keep waiting" arm at golden ckernel.cpp:148 (`return
    // false`), because :142's disjunction
    //   `fShuttleMove->bShuttleRetry || Sen[SnFKRetry].IsOn() || Sen[SnRKRetry]
    //    .IsOn()`
    // then rests entirely on the two physical retry keys.  true would instead
    // take :144-146 on the FIRST call, auto-confirming an operator "Retry"
    // press that nobody made -- every manual-retry wait in the machine would
    // fall straight through.  false is also what golden itself holds this flag
    // at whenever the form is not being clicked: the ctor sets it false
    // (ShuttleMove.cpp:63) and FormShow re-clears it on every open
    // (ShuttleMove.cpp:87).  Its ONLY setter in all of golden is the UI click
    // handler btRetryClick (ShuttleMove.cpp:1405-1407, `fShuttleMove->
    // bShuttleRetry=true;`), which has no offline caller -- so offline the
    // flag stays false until something in a future UI wave sets it, which is
    // the faithful behaviour, not a convenience.
    bool bShuttleRetry;

    // [WIDGET] golden ShuttleMove.h:40 `TButton *btRetry;` (confirmed a real
    // TButton by the .dfm too: golden ShuttleMove.dfm:787 `object btRetry:
    // TButton`).  Reuses the stock stand-in from vclcompat/Controls.h, reached
    // through forms/FormWidgets.h -- no new widget SHAPE is needed and
    // FormWidgets.h is NOT edited.  Spelled `vclcompat::TButton` rather than
    // adding a `TfShuttleMoveButton` typedef next to Part 1's per-form
    // typedefs: those typedefs live in FormWidgets.h (which this wave must not
    // touch), and the unqualified `TButton` is in any case already pulled to
    // global scope by Controls.h:484, so the qualified spelling here is purely
    // to make the provenance unmistakable at the declaration.
    //
    // OFFLINE DEFAULTS, per member of the stand-in:
    //  * ->Caption defaults to "" (vclcompat::TButton's default-constructed
    //    AnsiString).  This selects NO branch and cannot: ckernel.cpp:138/:140
    //    only ever WRITE it -- both arms of the if/else assign before anything
    //    could read -- and no ported code reads btRetry->Caption at all.  Note
    //    golden's DESIGN-TIME value is 'Retry' (ShuttleMove.dfm:792); it is
    //    deliberately NOT replicated as a member initialiser here, because
    //    vclcompat/Controls.h's DEFAULT-VALUE RULE puts .dfm values in the
    //    W7-B1c layout table, not in C++ initialisers.  Setting it would create
    //    a divergence from that rule while changing nothing observable.
    //  * ->Visible defaults to false (inherited TControl).  Recorded because
    //    this is one of the rare cases where the facade's blanket-false default
    //    happens to be VCL-FAITHFUL rather than merely conservative: golden's
    //    .dfm has `Visible = False` (ShuttleMove.dfm:794) and FormShow forces
    //    it false in the normal build (`#else btRetry->Visible=false;`, golden
    //    ShuttleMove.cpp:96-97) -- it is only made visible under SOFT_SIMULTE
    //    (ShuttleMove.cpp:94-95).  Nothing in ckernel.cpp reads it either way.
    vclcompat::TButton *btRetry;

    // [DATA] golden ShuttleMove.h:96 `bool fShow;` -- "is this form on screen".
    //
    // OFFLINE DEFAULT false -- THIS IS A BRANCH SELECTION, stated explicitly,
    // in the same voice as bShuttleRetry above.
    //
    // WHY false IS THE FAITHFUL VALUE, NOT MERELY THE CONVENIENT ONE.  There is
    // no VCL form on screen in this target at all: the architecture (user's
    // 20260812 ruling) is web UI + C++ core, and wb_serve/the tests run with no
    // window of any kind.  So "the Shuttle Move form is not showing" is not an
    // assumption -- it is the only thing that can be true here.  The tree already
    // holds two SIBLING flags at exactly this value for exactly this reason and
    // says so at their declarations: fContact->fShow (atester_shims.h:157,
    // "offline false") and fTemp_Set->fShow (forms/fTemp_Set.h:505).  Both are
    // LIVE -- WebStart.cpp already reads them in translated, compiled code -- so
    // this member makes the family consistent rather than introducing a new rule.
    //
    // WHAT IT UNBLOCKS, AND WHAT IT DOES NOT.  golden reads this flag as the
    // "...unless the operator has that window open" exception inside && chains
    // (e.g. golden :5929, :6169).  With false those chains take golden's NORMAL
    // arm -- the one that runs when nobody is standing at that form.  It does
    // NOT follow that every gate reading it may now be lifted: where false is
    // the PROTECTIVE arm rather than the normal one, lifting the gate would mean
    // never protecting.  Each gate is judged on its own; see
    // docs/ST_FSHOW_SIMPLE_PROPOSAL.md.
    //
    // WHEN THE WEB LAYER STARTS REPORTING PAGE VISIBILITY, this is one of the
    // flags it writes -- and then every reader changes behaviour together and
    // consistently, with no second visit to the call sites.
    bool fShow;

    // AI(W906-SM-READDATA) 20260925 (Steven 團隊, C 路 ShuttleMove): [METHOD] golden ShuttleMove.h:105
    // `void ReadData();` -- body golden ShuttleMove.cpp:2252-2264, translated verbatim in
    // forms/fShuttleMove.cpp.  Reads <recipe>\HandlerCondition.Data [Shuttle]:
    //   InSH1SenICAddPos / InSH2SenICAddPos -> Prod.iInSH1SenICAddPos / iInSH2SenICAddPos
    //   InShtLtcPercent                     -> iInShtLtcPercent below (golden: fCCLink->iInShtLtcPercent)
    // golden callers: main.cpp:11741 (TfMain::FormShow), main.cpp:25814 (TfMain::ChangeSetUpFile),
    // cinitial.cpp:11230 (SetTechDataToProd_Shuttle), ShuttleMove.cpp:111 (FormShow) and :1988
    // (sbUpdateClick) -- the last two now reach it from FileRW/ShuttleMove.gen.inc.
    // Pure read: ReadIniData(int) never seeds a missing key (common.cpp:851-861), so calling it
    // writes nothing.
    void ReadData();

    // [DATA] stand-in for golden fCCLink->iInShtLtcPercent (golden CCLink/MyCCLinkSensor.h:445,
    // ctor default 80 at MyCCLinkSensor.cpp:42).  The TfCCLink form is not ported, so its field has
    // no home; it lives here next to its only ported writer (ReadData above).  Golden readers:
    // ShuttleMove.cpp:2623/:2625 (DoInShuttleChkStackAction, the latch auto-teach flow -- NOT
    // ported) and MyCCLinkSensor.cpp:1584-1585/:1683 (TfCCLink's own read/save -- NOT ported).
    // So today nothing in this tree reads it; when TfCCLink lands, move it there.
    int iInShtLtcPercent;

    TfShuttleMove();
    virtual ~TfShuttleMove() {}
};
extern TfShuttleMove *fShuttleMove;   // golden: extern PACKAGE TfShuttleMove *fShuttleMove; (ShuttleMove.h:121)

#endif // FORMS_FSHUTTLEMOVE_H
