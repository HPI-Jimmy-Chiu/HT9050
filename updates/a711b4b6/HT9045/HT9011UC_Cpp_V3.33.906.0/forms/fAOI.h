// =============================================================================
//  forms/fAOI.h  --  non-VCL stand-in for golden's FrmAOI form pointer
//
//  AI(W906-W7-L2) 20260803: FIRST home for TFrmAOI anywhere in this migrated
//  tree (golden fAOI.h:25 `class TFrmAOI : public TForm`).  Landed by the W7-L2
//  substrate pass ahead of the translation of golden ckernel.cpp.
//  forms/fNote.{h,cpp} convention; the forms/fMain.h facade contract binds this
//  file too.
//
//  FrmAOI has been NAMED in this tree before without being landed --
//  aoutarm9045.cpp:711/:714/:802 record an out-arm Rotate/AOI/FixAI predicate
//  left inside `#if 0 // TODO(W7)` partly BECAUSE FrmAOI had no home.  This
//  facade does NOT un-gate that block (different wave, different file, and the
//  block needs tRotate / tAOISetup / ScannerAOIIF as well).  Reported, not done.
//
//  ---------------------------------------------------------------------------
//  CLASS NAME: `TFrmAOI`, NOT `TfAOI`.  DEVIATION FROM THE BRIEF, ON GOLDEN'S
//  AUTHORITY.
//  ---------------------------------------------------------------------------
//  The W7-L2 assignment asked for "TfAOI / global FrmAOI".  Golden fAOI.h:25 is
//  `class TFrmAOI : public TForm`, fAOI.h:414 is `__fastcall TFrmAOI(TComponent*
//  Owner);` and fAOI.h:492 is `extern PACKAGE TFrmAOI *FrmAOI;`.  Golden wins.
//  Same deviation, same reason, as forms/fRotate.h's TFrmRotate.
//
//  ---------------------------------------------------------------------------
//  ##  THE NULL QUESTION -- I DID NOT DO WHAT THE BRIEF ASKED.  HERE IS WHY.  ##
//  ---------------------------------------------------------------------------
//  The W7-L2 assignment stated that because golden null-checks this pointer at
//  ckernel.cpp:502 (`if(FrmAOI!=NULL)`), "a null global is the faithful offline
//  state and the class exists only so the guarded block compiles", and asked for
//  `FrmAOI = NULL`.  I checked that premise against golden instead of taking it,
//  and it does not hold:
//
//    golden HT9045.cpp:233
//        Application->CreateForm(__classid(TFrmAOI), &FrmAOI);
//
//  That line sits in the flat CreateForm list at HT9045.cpp:166-245, which I
//  read end to end: 80 consecutive CreateForm calls, no `if`, no config gate, no
//  machine-type switch anywhere in the block.  fMain (:166), fNote (:173), COM2
//  (:174), fHome (:175), fSetup (:183), fContact (:184), FrmRotate (:226),
//  Zteach (:230) and FrmAOI (:233) are all created the same unconditional way.
//  So in golden, FrmAOI is non-NULL for the entire life of the process after
//  WinMain, and ScanSystemSensor -- which can only run once SoftStart has been
//  set from the UI, i.e. long after WinMain -- ALWAYS takes the true arm at
//  :502 and ALWAYS executes :504.  The `!=NULL` guard is defensive coding added
//  by the same 20260515 change that introduced the field (both lines carry the
//  "Eastsun 20260515 F023" tag), not evidence of a reachable NULL state.
//
//  Landing NULL would therefore have made ckernel.cpp:504 permanently dead code
//  in the ported tree, unreachable by any test, while the translation claimed to
//  be faithful.  That is the specific failure mode this project has been burned
//  by.  So forms/fAOI.cpp allocates a live instance, and golden's `if(FrmAOI!=
//  NULL)` guard must still be transcribed VERBATIM by the ckernel translation --
//  it is golden's line, it simply evaluates true, exactly as it does on a real
//  machine.
//
//  RISK OF MY CHOICE, STATED: none that I can find.  See bSimulateTopBtm below
//  -- the field has no reader anywhere in golden, so executing :504 changes
//  nothing observable; it only makes the branch live and testable.
//
//  ---------------------------------------------------------------------------
//  GOLDEN ckernel.cpp CENSUS FOR FrmAOI -- MEASURED OVER ALL 2589 LINES
//  ---------------------------------------------------------------------------
//    :46   #include "fAOI.h"                (the include golden added for this)
//    :502  if(FrmAOI!=NULL)                 (read)  ScanSystemSensor
//    :504  FrmAOI->bSimulateTopBtm=true;    (write) ScanSystemSensor
//  One member.  Golden's TFrmAOI is a 784-line header with a large inspection
//  API; none of the rest is reachable from ckernel.cpp and none is landed.
// =============================================================================
#ifndef FORMS_FAOI_H
#define FORMS_FAOI_H

// =============================================================================
//  AI(W906-FW3-AOI1) 20260827: WIDENED past "one bool" (the line above used to
//  say "dependency-free header: one bool", true until this wave). FW3-AOI1 is
//  the first read-only translation pass over golden fAOI.cpp (8,525 lines /
//  174 census-countable functions -- measured this wave with
//  tools/census/coverage_probe.py + span_sanity.py, both 174/174, 0 missed).
//  15 functions landed; the other 159 are hardware motion, AOI-host TCP
//  commands, file saves, alarms, or (one case, GetAOIFailBin) a link-boundary
//  block -- excluded by design, not oversight (see the per-function
//  reasoning in forms/fAOI.cpp). Landing even this small a slice needs:
//    - vclcompat/vcl_compat.h  for AnsiString (ShowTopBtnTask's parameter).
//    - vclcompat/Controls.h    for the handful of __published widget types
//      the 15 functions touch (golden fAOI.h:28-361) -- TEdit/TComboBox/
//      TCheckBox/TRadioGroup/TLabel/TButton, all pre-existing vclcompat
//      classes (see forms/fContact.h's "five facade-local widget stand-ins"
//      note for the established pattern this follows: `new TWidget()`
//      in-class default member initializers, no ctor changes needed).
//    - cprod.h                 for Prod (read-only in CheckFailBin).
//  GetAOIFailBin (golden fAOI.cpp:2892-2946) was drafted, syntax-checked, and
//  then WITHDRAWN: it needs OutArmSuck.iBinData, and OutArmSuck's only live
//  definition (aHotPlateSubstrate.cpp:83) is compiled into ht9045_sm, which
//  ht9045_forms does not link (CMakeLists.txt:714, and the reverse edge is a
//  documented target cycle). ScannerAOIIF/TrayForm/Prod are all fine (defined
//  in cprod.cpp, ht9045_globals) -- OutArmSuck alone is the blocker. See
//  forms/fAOI.cpp for the full note.
//  This header still carries ZERO of golden's 118 __published widgets as a
//  real form (no .dfm, no PageControl/GroupBox/TabSheet container tree) --
//  only the 14 leaf controls the 15 landed functions actually dereference.
// =============================================================================
#include "vclcompat/vcl_compat.h"    // AnsiString (+ transitively TStringList etc)
#include "vclcompat/Controls.h"      // TEdit/TComboBox/TCheckBox/TRadioGroup/TLabel/TButton
#include "cprod.h"                   // Prod (CheckFailBin's Prod.bIsPassBin[] read)

class TFrmAOI
{
public:
    // -----------------------------------------------------------------------
    //  [DATA] golden fAOI.h:472 `bool bSimulateTopBtm;`
    //  Golden's own comment on that line: "Eastsun 20260515 F023: E2 lock
    //  simulate during run" -- i.e. "the Top/Bottom AOI inspection is pinned to
    //  simulate mode because a lot is running".
    //
    //  OFFLINE VALUE false.  Golden's ctor `__fastcall TFrmAOI::TFrmAOI
    //  (TComponent* Owner)` (fAOI.cpp:128) does NOT assign it -- same VCL
    //  zero-fill situation as forms/fRotate.h's flags, so false is golden's real
    //  initial value.
    //
    //  THE BRANCH false SELECTS -- and an honest measurement that constrains how
    //  much that claim is worth.  I scanned ALL 887 files of the golden tree
    //  (every extension, not just .cpp/.h) for this identifier.  It appears
    //  exactly twice:
    //      fAOI.h:472      the declaration
    //      ckernel.cpp:504 the write, `FrmAOI->bSimulateTopBtm=true;`
    //  There is NO READER of bSimulateTopBtm anywhere in golden V3.33.906.0.
    //  As of this golden snapshot the field is write-only -- the F023 feature
    //  that consumes it has evidently not landed on the AOI side yet.  So the
    //  truthful statement is: false selects no golden branch today, because no
    //  golden code branches on it.  It is landed anyway because golden
    //  ckernel.cpp:504 writes it and a faithful ScanSystemSensor must perform
    //  that write; the field is the storage that write needs, and false is the
    //  correct pre-START value so a test can observe the transition false->true
    //  across SoftStart.  That observable transition is, right now, the entire
    //  behavioural content of golden ckernel.cpp:502-505.
    // -----------------------------------------------------------------------
    bool bSimulateTopBtm;

    // -------------------------------------------------------------------
    //  AI(W906-FW3-AOI1) 20260827: 14 widget stand-ins, ONLY the leaves the
    //  15 functions landed this wave touch (golden __published list is 118
    //  controls long, fAOI.h:28-361; this is not that list).  Pattern is
    //  forms/fContact.h's "five facade-local widget stand-ins" (D-4): plain
    //  vclcompat types, `new TWidget()` in-class default member
    //  initializers so the ctor below needs no changes (Gotcha #4: a
    //  facade ctor must not hand-roll construction of heap objects it
    //  doesn't already own -- these initializers run for every ctor
    //  automatically, same as fContact.h's edXDimension etc).
    // -------------------------------------------------------------------
    TComboBox   *ComboBox1             = new TComboBox();   // golden fAOI.h:89  (DoViewBinFunction)
    TEdit       *edt_FailPADView       = new TEdit();       // golden fAOI.h:48  (CheckFailBin)
    TEdit       *edt_FailBGAView       = new TEdit();       // golden fAOI.h:85  (CheckFailBin)
    TEdit       *edt_PassFailBGAPADView= new TEdit();       // golden fAOI.h:91  (CheckFailBin)
    TEdit       *edt_FailFailBGAPADView= new TEdit();       // golden fAOI.h:92  (CheckFailBin)
    TCheckBox   *cbEnabledTopView      = new TCheckBox();   // golden fAOI.h:36  (CheckFailBin)
    TCheckBox   *cbEnabledPADView      = new TCheckBox();   // golden fAOI.h:47  (CheckFailBin)
    TCheckBox   *cbEnabledBGAView      = new TCheckBox();   // golden fAOI.h:84  (CheckFailBin)
    TRadioGroup *rgAOIFailBinType      = new TRadioGroup(); // golden fAOI.h:248 (UpdateAOIFailBinTypetoForm)
    TLabel      *lblAOIBinSel1         = new TLabel();      // golden fAOI.h:249 (UpdateAOIFailBinTypetoForm)
    TLabel      *lblAOIBinSel2         = new TLabel();      // golden fAOI.h:250 (UpdateAOIFailBinTypetoForm, ->Visible)
    TComboBox   *cbAOIFialAndTestPass  = new TComboBox();   // golden fAOI.h:251 (UpdateAOIFailBinTypetoForm, ->Visible only -- "cb" prefix but golden types it TComboBox, not TCheckBox)
    TButton     *btnSimulateTopBtm     = new TButton();     // golden fAOI.h:340 (SetSimulationState)
    TLabel      *blTopBtmTask          = new TLabel();      // golden fAOI.h:361 (ShowTopBtnTask)

    // -------------------------------------------------------------------
    //  AI(W906-FW3-AOI1) 20260827: golden fAOI.cpp:3715-3736, pure -- reads
    //  4 TEdit->Text + 3 TCheckBox->Checked + Prod.bIsPassBin[] (global
    //  production state, READ only), returns bool, writes nothing.  Golden
    //  quirk preserved verbatim: `int iBin[5]` is indexed 1..4 only, iBin[0]
    //  is declared but never read or written (see forms/fAOI.cpp for the
    //  loop bound `i<sizeof(iBin)/sizeof(int)` starting at i=1).
    // -------------------------------------------------------------------
    bool CheckFailBin();

    // AI(W906-FW3-AOI1) 20260827: golden fAOI.cpp:4472-4483 + inline getter
    // at golden fAOI.h:445.  bSimulate has NO ctor-assigned default in this
    // facade's golden span (golden only sets it inside the
    // `if(USE_Scanner_AOI_Inspection==...)` gate of TFrmAOI's own ctor,
    // fAOI.cpp:152, which this facade's ctor does not reproduce -- see
    // forms/fAOI.cpp for why).  false is chosen explicitly here (not left to
    // implicit init) as the safer of golden's two live values; same
    // reasoning class as bSimulateTopBtm above.
    bool bSimulate = false;
    void SetSimulationState(bool bswitch=false);
    bool GetSimulationState(){return bSimulate;}

    // AI(W906-FW3-AOI1) 20260827: golden fAOI.cpp:4516-4519, a single
    // Caption write.  Landed but currently unwired -- its only golden
    // caller is TTopBottomInspect::DoTopBtmInspFunc_Inspection (motor+socket
    // state machine, excluded this wave; see forms/fAOI.cpp).
    void ShowTopBtnTask(AnsiString sMsg);

    TFrmAOI();
    virtual ~TFrmAOI() {}

private:
    // AI(W906-FW3-AOI1) 20260827: golden fAOI.cpp:4429-4443, private in
    // golden too (fAOI.h:408).  Pure widget-to-widget update, no config
    // dependency, no landmine -- but its only golden caller,
    // rgAOIFailBinTypeClick, is a *Click event handler and is excluded by
    // the hard "no event handlers wired" rule.  Landed standalone,
    // reachable only by a future wave that ports the click handler.
    void UpdateAOIFailBinTypetoForm();
};

// golden: extern PACKAGE TFrmAOI *FrmAOI;   (fAOI.h:492)
extern TFrmAOI *FrmAOI;

// -----------------------------------------------------------------------------
//  AI(W906-FW3-AOI1) 20260827: subset of golden fAOI.h's extern block
//  (fAOI.h:494-529) covering ONLY the globals the 8 free functions landed
//  this wave read or write.  Everything golden declares extern there that
//  belongs to an EXCLUDED function (iAOITask/DoAOIFunction/InitAOIFunction,
//  bTriggerAOI, iAOIResult, iScanAOIIntervalCounter, the *ByArm*/*BySite*
//  continuous-fail family -- see forms/fAOI.cpp for why those are excluded)
//  is deliberately NOT reproduced here, to avoid declaring a global with no
//  live definition anywhere in the port.
//
//  iScanLGAStep / bplus / irowX / icolumnY / iTopScannerAOIInspectionTask /
//  iTopScanAOITask are likewise real globals (defined in forms/fAOI.cpp,
//  same as golden fAOI.cpp:104-125) but golden itself does NOT extern-
//  declare them in fAOI.h either -- file-private-by-convention in golden,
//  matched here for fidelity rather than widened.
// -----------------------------------------------------------------------------
extern int iTopViewTask;
extern int iTopViewInspectionTask;
extern int iPADViewTask;
extern int iPADViewInspectionTask;
extern int iBGAViewTask;
extern int iBGAViewInspectionTask;
extern int iScanAOITask;
extern int iScannerAOIInspectionTask;

// -----------------------------------------------------------------------------
//  //AI(W906-FRW-S69) 20260926: golden fAOI.cpp:63-64／:73-74 的四張 Dut on/off 表（Alick 20160713／20160714）。
//  golden 是 fAOI.cpp 檔案層級全域、fAOI.h 沒有 extern；移植樹要跨 TU：讀寫檔在 FileRW/AOISetup.gen.inc
//  （fAOI_ReadFile :3420-3431 讀 AOI.Data [DutOnOff_BGAView]／[DutOnOff_PADView]、spbSaveClick :3187-3204 寫），
//  AOI 執行期（DoContinuousFailBy*、DoBGAView*、DrawSitePanelVitrox*、mtDutOnOff_* 點格子，Jimmy）將來在本檔翻 ——
//  兩邊必須是同一份，所以定義放 golden 的家（forms/fAOI.cpp），這裡只宣告。零初值同 golden（全域）。
//  （上面的「只 extern 本檔已落地函式用到的全域」原則照舊：這四張表的讀寫者已在 FileRW/AOISetup.gen.inc 落地。）
// -----------------------------------------------------------------------------
extern bool bVitroxBGAViewMap[MAX_ARM_Row][MAX_ARM_Col];                      // golden fAOI.cpp:63
extern bool bVitroxBGAViewUse[MAX_ARM_Row][MAX_ARM_Col];                      // golden fAOI.cpp:64
extern bool bVitroxPADViewMap[MAX_ARM_Row][MAX_ARM_Col];                      // golden fAOI.cpp:73
extern bool bVitroxPADViewUse[MAX_ARM_Row][MAX_ARM_Col];                      // golden fAOI.cpp:74

extern void InitTopScannerAOIInspection();                                     // golden fAOI.h:527

// golden fAOI.h:529 `extern int GetAOIFailBin(int iRow, int iCol);` is
// DELIBERATELY NOT reproduced here -- drafted this wave, then withdrawn for
// a link-boundary block (OutArmSuck lives in ht9045_sm; see the header
// banner above and forms/fAOI.cpp).

#endif // FORMS_FAOI_H
