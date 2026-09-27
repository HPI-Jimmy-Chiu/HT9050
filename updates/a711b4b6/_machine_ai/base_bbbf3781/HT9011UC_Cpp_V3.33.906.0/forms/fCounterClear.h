// =============================================================================
//  forms/fCounterClear.h  --  non-VCL facade for golden's TfCounterClear
//                              (cCounterClear.h)
//
//  AI(W906-FW-SecCC) 20260819: new file, FW-3 queue item "FW-SecCC".
//  Golden ref: HT9011UC_Code_V3.33.906.0_20260618/cCounterClear.h (48 lines)
//  + cCounterClear.cpp (504 lines, cp950/Big5 -- decoded with `python3 -c
//  "open(path,'rb').read().decode('cp950')"`, 0 U+FFFD, this wave).
//
//  ROLE
//  ----
//  TfCounterClear is golden's "Counter Clear" dialog: 8 checkboxes (Alarm
//  Data / Test Category / Scanner / Loading Count / Contact Count Curr+His /
//  Sorting Count / Time Data) gated by cAuthority.h's authCounterClr[8], plus
//  the actual clearing logic (ClearCount(ClearType), one switch arm per
//  counter family) that OTHER files call directly, bypassing the dialog
//  entirely, at 17 call sites tree-wide (see GATE REGISTER). ClearCount is
//  therefore the load-bearing member -- the dialog UI is a thin wrapper
//  around it (spbExeClick just walks the 8 checkboxes and calls ClearCount
//  once per checked box; AutoClear does the same driven by authCounterClr[]
//  instead of a checkbox).
//
//  FORM CLASS SHAPE (matches forms/fContactCT.h / forms/fShowBinSelect.h
//  precedent: plain non-VCL class, NOT TForm-derived -- see those files'
//  banners for why; `grep -n "^class Tf" forms/*.h` confirms every existing
//  translated form facade in this tree follows the same shape).
//
//  TRANSLATION STATUS (every golden method, golden line span)
//  --------------------------------------------------------------------------
//    TfCounterClear()          golden :24-28    FULL  (bHasFile=true; only --
//                                                zero cross-TU/config touch,
//                                                see ctor safety note below)
//    WriteCTInfo               golden :30-38    FULL  (ArmData/ArmDataLot/
//                                                ArmHistory ->WriteFile(),
//                                                all real -- cSocket.h:177-178)
//    ReadCTInfo                golden :40-48    FULL  (symmetric ->ReadFile())
//    cbSelectAllMouseUp        golden :50-77    FULL
//    cbAlarmDataMouseUp        golden :79-88    FULL
//    FormShow                  golden :92-114   FULL
//    ClearCount                golden :116-392  FULL, 1 WRITE-PATH GATE
//                                                (ctTesterCategory's
//                                                DeleteFile(BatFile), BatFile is
//                                                "D:\HT9045\system\BinCount.txt"
//                                                -- system\ is
//                                                on the task's explicit
//                                                write-gate list); 3 FACADE
//                                                GATES (fProductionInfo->
//                                                CalculateNowArmSiteBinQty /
//                                                CalculateNowUnloaderTrayQty /
//                                                UpdateControlBinCount -- none
//                                                exist on forms/
//                                                fProductionInfo.h's
//                                                TfProductionInfo, see GATE
//                                                REGISTER)
//    spbExeClick               golden :394-450  FULL, 1 FACADE GATE (the
//                                                CC_KYEC_LEE branch touches
//                                                fMain->cbUserSelect/
//                                                cbUserSelectChange/
//                                                stOperatorClick/btLogin/
//                                                spbUserName -- none exist on
//                                                forms/fMain.h)
//    spbExitClick              golden :452-456  FULL, DEVIATION (Close() ->
//                                                FormClose(), see below)
//    AutoClear                 golden :458-485  FULL
//    FormClose                 golden :487-491  FULL, DEVIATION signature
//                                                (Sender/TCloseAction dropped)
//    LowYieldSpecialInitail    golden :493-503  FULL
//
//  DEVIATION -- FormClose/spbExitClick signature and Close()
//  --------------------------------------------------------------------------
//  golden: `void __fastcall FormClose(TObject *Sender, TCloseAction &Action)`.
//  `TCloseAction` has no port anywhere in this tree (0 hits for a translated
//  type; the only tree-wide hits are golden-line-number comments in other
//  forms' headers, e.g. forms/fTemperFrom.h's own identical DEVIATION note
//  for the same reason). Dropped both parameters, matching that precedent.
//  `spbExitClick`'s golden body is `spbExit->Down=false; Close();` -- `Close()`
//  is VCL TForm::Close, which (for a form with no side effects on the OS
//  window itself, since this facade has no window) reduces to "run this
//  form's own FormClose", exactly the established idiom at
//  ATCInterface.cpp:446/:513 (`FormClose(); //GATE (1): golden 'Close()' ...
//  the ONLY observable effect golden's own FormClose has ... is applied
//  directly`). Translated as `FormClose();`.
//
//  CTOR SAFETY (the "SIOF homecoming" question -- see forms/fSecurity.h for
//  the contrast case)
//  --------------------------------------------------------------------------
//  golden ctor body, verbatim and in full: `bHasFile=true;`. That is the
//  WHOLE thing -- one assignment to a private bool member, no cross-TU global
//  read/write, no file I/O, no config dependency of any kind. This is
//  EXACTLY the "cContactCT-shape" ctor (see forms/fContactCT.h's own global
//  homecoming note: "this facade's ctor is a plain field bootstrap (no
//  config/file I/O, no INI read) -- so static-init safety is NOT the
//  blocker"), NOT the "cObserver/cShowBinSelect-shape" ctor that needs an
//  `if (INIFileGeneral != 0)` guard around part of its body. There is no
//  config-touching statement anywhere in this ctor to guard. The global is
//  therefore homecomed the SAME way cContactCT.cpp:94 already did it:
//  unconditional `new TfCounterClear()` at static-init time, no guard.
//
//  GATE REGISTER (grep evidence, run 20260819 against the working tree,
//  excluding build_*/ directories)
//  --------------------------------------------------------------------------
//  (CC1) ClearCount's `DeleteFile(BatFile)`, BatFile is
//        "D:\\HT9045\\system\\BinCount.txt" (golden :118/:146-152) -- `system\`
//        is on the task
//        brief's explicit write-gate list (setup.inf/CurrentSetupData.txt/
//        system\/config\/CFG\/IniData\ -> #if 0 unconditionally). Gated the
//        try/DeleteFile/catch block; the surrounding TastCategory.
//        UpdataCount(true) / MyDBIProcess / fProductionInfo->
//        UpdateControlBinCount(true) stay ACTIVE (UpdateControlBinCount is
//        itself GATE (CC2), see below -- two independent gates on the same
//        `case ctTesterCategory:` arm, each documented at its own site).
//  (CC2) ClearCount's `fProductionInfo->CalculateNowArmSiteBinQty(true)`
//        (golden :125), `fProductionInfo->UpdateControlBinCount(true)`
//        (golden :154), and `fProductionInfo->CalculateNowUnloaderTrayQty
//        (true)` (golden :208) -- forms/fProductionInfo.h's TfProductionInfo
//        facade declares exactly two members, `sLoadMO_TestFlow` and
//        `CalTrayICCount` (confirmed by reading the file in full, 20260819);
//        none of these three methods exist on it. Each call site is gated
//        individually with a pointer back to this note; nothing else in the
//        surrounding switch arm depends on them.
//  (CC3) spbExeClick's CC_KYEC_LEE branch (golden :396-418) touches
//        `fMain->cbUserSelect` (ItemIndex/Text), `fMain->ChangeLevelAttr()`
//        (this ONE call IS real -- forms/fMain.h:231), `fMain->spbUserName->
//        Caption`, `fMain->btLogin->Caption`, `fMain->cbUserSelectChange`,
//        `fMain->stOperatorClick`. `grep -n "cbUserSelect\|spbUserName\|
//        btLogin\|stOperatorClick\|cbUserSelectChange" forms/fMain.h` --
//        0 hits for all five, 20260819. Gated the WHOLE branch as a unit
//        (same "don't silently narrow a compound/sequential golden block"
//        posture as Command.cpp's FW3-WF GATE REGISTER item 4) rather than
//        picking out `ChangeLevelAttr()` alone, since the branch's own early
//        `return` (`if(AccessLevel<iDefSupervisorLevel) return;`) depends on
//        `AccessLevel`, which this gated branch itself sets via the missing
//        widgets -- there is no faithful partial evaluation.  DEFAULT: for
//        CC_KYEC_LEE, spbExeClick's KYEC re-auth prompt does not run, so
//        every checkbox already checked by the operator still gets cleared
//        (same "this port has no modal dialog surface yet" default as
//        forms/fContactCT.h GATE C4's sibling gates).
//
//  STUB COLLISION (searched 20260819, excluding build_*/ and golden dirs)
//  --------------------------------------------------------------------------
//  `class TfCounterClear` / `TfCounterClear *fCounterClear` (real type) --
//  0 hits anywhere in the port tree before this file. `fCounterClear` (the
//  BARE NAME, any type) has exactly ONE prior stand-in, and it is TU-local:
//  csystem.cpp:4851-4854's `#define fCounterClear W7C1_fCounterClear` (a
//  `struct W7C1_TfCounterClearSeam { void LowYieldSpecialInitail(){} };`
//  instance), preprocessor-scoped to csystem.cpp only (a `.cpp`-local
//  `#define`, invisible to every other translation unit -- confirmed `grep
//  -rn "fCounterClear" --include=*.h .` returns 0 hits tree-wide). See
//  cCounterClear.cpp's file-tail note for the retirement recommendation this
//  seam needs once the main loop registers this file.
// =============================================================================
#ifndef FORMS_FCOUNTERCLEAR_H
#define FORMS_FCOUNTERCLEAR_H

#include "vclcompat/vcl_compat.h"   // AnsiString, TObject
#include "vclcompat/Controls.h"     // TCheckBox, TSpeedButton, TGroupBox

using vclcompat::TObject;
using vclcompat::TCheckBox;
using vclcompat::TSpeedButton;
using vclcompat::TGroupBox;

// =============================================================================
//  TfCounterClear -- non-VCL facade (golden cCounterClear.h)
// =============================================================================
class TfCounterClear
{
public:
    // -- widgets (golden __published, cCounterClear.h:13-25) -----------------
    TGroupBox   *gbItems             = new TGroupBox();
    TCheckBox   *cbAlarmData         = new TCheckBox();
    TCheckBox   *cbTestCategory      = new TCheckBox();
    TCheckBox   *cbScanner           = new TCheckBox();
    TCheckBox   *cbLoadingCount      = new TCheckBox();
    TCheckBox   *cbContactCountCurr  = new TCheckBox();
    TCheckBox   *cbContactCountHis   = new TCheckBox();
    TCheckBox   *cbSortingCount      = new TCheckBox();
    TCheckBox   *cbTimeData          = new TCheckBox();
    TCheckBox   *cbSelectAll         = new TCheckBox();
    TSpeedButton *spbExe             = new TSpeedButton();
    TSpeedButton *spbExit            = new TSpeedButton();

    // -- data (golden cCounterClear.h:35, 42) ---------------------------------
    bool fShow = false;

    TfCounterClear();

    void WriteCTInfo();
    void ReadCTInfo();

    void cbSelectAllMouseUp(TObject *Sender);
    void cbAlarmDataMouseUp(TObject *Sender);
    void FormShow(TObject *Sender);
    void ClearCount(int ClearType);
    void spbExeClick(TObject *Sender);
    void spbExitClick(TObject *Sender);
    void AutoClear();
    // DEVIATION: golden `void __fastcall FormClose(TObject *Sender,
    // TCloseAction &Action)` -- TCloseAction has no port, see banner.
    void FormClose();
    void LowYieldSpecialInitail();

private:
    // golden cCounterClear.h:35 -- private, never read by anything this wave
    // translates (checked: no method body references bHasFile). Carried for
    // structural fidelity only.
    bool bHasFile = false;
};

// AI(W906-FW-SecCC) 20260819: global homecomed unconditionally -- this ctor
// is a plain field bootstrap (see CTOR SAFETY note above), the same shape as
// cContactCT.cpp:94's already-homecomed `new TfContactCT()`, not the guarded
// cObserver/cShowBinSelect shape.
extern TfCounterClear *fCounterClear;   // golden cCounterClear.h:46 (extern PACKAGE TfCounterClear *fCounterClear;)

#endif // FORMS_FCOUNTERCLEAR_H
