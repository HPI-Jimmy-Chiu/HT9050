// =============================================================================
//  forms/fYieldMonitoring.h  --  non-VCL facade for golden's TfYieldMonitoring
//  (uYieldMonitoring.h)
//
//  AI(W906-FW3-YieldMon-WA) 20260818: new file, FW-3 uYieldMonitoring Wave A.
//  Golden ref: HT9011UC_Code_V3.33.906.0_20260618/uYieldMonitoring.h (627
//  lines) + uYieldMonitoring.cpp (5,981 lines, cp950 -- decoded with
//  `python3 -c "open(...,'rb').read().decode('cp950')"`, 0 U+FFFD, this wave).
//
//  ROLE
//  ----
//  TfYieldMonitoring is golden's Yield/Auto-Site-Off *setup dialog*, but the
//  17 methods this wave translates are its PURE YIELD ENGINE CORE -- the
//  per-tick site/picker/total/interval/special low-yield alarm checks, the
//  sliding-window ring buffer, and two small pure helpers (CheckSettingNo,
//  SetClosedSiteBin). None of them read or write a widget except
//  SetClosedSiteBin's one TComboBox (cbbClosedSiteBin) -- this class is, for
//  Wave A's purposes, almost pure data + logic, unlike TfObserver's
//  grid/tray/chart-heavy facade (forms/fObserver.h).
//
//  THIS IS A SEPARATE, NEW CLASS -- NOT THE EXISTING STAND-IN
//  --------------------------------------------------------------------------
//  aHotPlateSubstrate.h:1146-1150 ALREADY declares a class
//  `TfYieldMonitoring_2x4_16` and a global `TfYieldMonitoring_2x4_16
//  *fYieldMonitoring` (golden uYieldMonitoring.h's own `extern PACKAGE
//  TfYieldMonitoring *fYieldMonitoring;`), landed by an earlier wave so the
//  2x4_16 in-arm variant's `fYieldMonitoring->DoAutoCloseSite(bool)` call
//  binds to something. This header does NOT touch that global (no
//  `fYieldMonitoring` declared here, not even `extern`) -- the class name
//  `TfYieldMonitoring` below is DIFFERENT from `TfYieldMonitoring_2x4_16`, so
//  the two coexist without ODR contact (same posture as forms/fObserver.h's
//  TfObserver vs. atester_shims.h's TfObserverShim). Swapping the live global
//  to point at a real TfYieldMonitoring instance is a main-loop INTEGRATION
//  decision, not this wave's -- see docs/KNOWLEDGE.md's two-TMyKitSuck-headers
//  entry for the same class of decision. Until that swap, `class
//  TfYieldMonitoring` below has no live instance anywhere in the tree; it is
//  exercised only by tests/test_yieldmon_core.cpp (this wave), which
//  constructs its own local instance.
//
//  NO CONSTRUCTOR THIS WAVE (deliberate, not an oversight)
//  --------------------------------------------------------------------------
//  Golden's TfYieldMonitoring(TComponent*) ctor and FormCreate/DoIniDataToForm
//  are explicitly OUT OF SCOPE this wave (Wave C, per the task brief). Rather
//  than write a partial/fake ctor, every data member below carries an in-class
//  default member initializer (NSDMI) so the IMPLICITLY-GENERATED default
//  constructor already does the right thing -- zero-valued scalars/arrays,
//  and the one widget pointer (cbbClosedSiteBin) heap-allocated inline. This
//  is the SAME convention forms/fObserver.h's own FW-3 Wave 2 addendum
//  established for members added without their golden ctor in scope (see that
//  header's "NEW FACADE MEMBERS this wave's methods dereference" banner). No
//  user-declared destructor either -- the one leaked TComboBox is an
//  acceptable process-lifetime leak, identical in kind to fObserver.h's own
//  Wave-2 pointer additions (same rationale: at most one instance per process
//  in every real consumer, real future UI wiring and this wave's own tests).
//
//  WAVE SCOPE (every golden method this header declares, golden line span)
//  --------------------------------------------------------------------------
//    CalculateSiteYield                  golden :3639-3808  (2 fContactCT + fLotInfo GATEs, see below)
//    CheckBySiteYieldAlarm               golden :3809-4032  (2 fContactCT GATEs)
//    CheckByPickerYieldAlarm             golden :4033-4255  (1 fShowBinSelect GATE)
//    ClearAutoSiteOffStatus              golden :4256-4267
//    CheckBySiteByArmYieldAlarm          golden :4268-4521  (2 fShowBinSelect GATEs)
//    CheckLowYieldAlarm                  golden :4522-4956  (17 fContactCT + 2 fShowBinSelect GATEs)
//    CheckLowYieldAlarmByTotal           golden :4957-5102  (2 fShowBinSelect touches, ACTIVE -- see DESIGN NOTE)
//    SWRingReset/SWRingPush/CheckLowYieldAlarm_SW  golden :5105-5159
//    ClearYieldCount                     golden :5161-5200
//    CanAutoCloseSite                    golden :5300-5334  (pure judgement, no gates)
//    CheckIntervalLowYieldAlarmBySite    golden :5490-5592  (1 fContactCT GATE)
//    CheckIntervalLowYieldAlarmByTotal   golden :5627-5701
//    CheckLowYieldAlarmSpecial           golden :5703-5805
//    CheckSettingNo                      golden :3220-3231  DEVIATION: public (golden private, see below)
//    SetClosedSiteBin                    golden :5972-5980
//
//  EXPLICITLY EXCLUDED (per task brief, not even a stub declared here)
//  --------------------------------------------------------------------------
//  Save*/btn*Click, FormCreate/FormShow/DoIniDataToForm/
//  DoFormToData (Wave C), TMyYieldPanel (Wave B, cObserver-adjacent UI panel).
//  [HISTORY] DoAutoCloseSite / DoRTAutoSocketOff sat here as SAFETY ACTIONS
//  until the user approved the queue (20260817 ruling); real bodies landed
//  20260818 as queue item 3 (AI(W906-FW-Q3), see method decls below).
//
//  DEVIATION -- CheckSettingNo made public
//  --------------------------------------------------------------------------
//  Golden declares it `private` (uYieldMonitoring.h:542). Same rationale as
//  every other translated form facade in this tree (forms/fObserver.h's own
//  banner: "no BCB6 private/__published split" convention) -- BCB6's
//  private/__published split has no faithful C++17 analogue once the .dfm
//  streaming machinery that justified it is gone, and tests/
//  test_yieldmon_core.cpp needs to call it directly.
//
//  GATE REGISTER
//  --------------------------------------------------------------------------
//  (Y1) fContactCT (golden cContactCT.h, TfContactCT*) -- COMPLETELY unported.
//      Confirmed absent: `grep -rn "class.*TfContactCT\|fContactCT *;"
//      --include=*.h .` over the whole port tree (excluding golden/build
//      trees) -- 0 hits for any TfContactCT type declaration or facade
//      global; cMyDB.cpp has 4 pre-existing TODOs citing the exact same gap
//      (grepped 20260818). csystem.cpp:4840-4850 DOES carry a TU-local
//      `W7C1_TfContactCTSeam` macro-`#define`d seam, but it is `static` +
//      file-scope-macro'd INSIDE csystem.cpp's own `#ifndef W7C1_SEAM` block
//      -- invisible to this TU, and it only covers 2 of the ~7 distinct
//      fContactCT members our 17 methods touch (rgYieldType, ClearData(int,
//      int); NOT GetLowYield_AutoClean/ReturnSiteDataArray/ReturnSiteData/
//      ClearData_AutoClean/sgYield/SaveTotalYield). Per the task brief,
//      EVERY fContactCT touch in this wave's scope is `#if 0`-gated in
//      uYieldMonitoring.cpp with a one-line pointer back to this entry,
//      rather than inventing a second, wider seam type (out of this wave's
//      "don't build UI infra" brief) -- 25 touches total, exact golden line
//      numbers verified this wave against a CRLF-normalized cp950-decoded
//      copy (`sed -n` line-number cross-check, 20260818): CalculateSiteYield
//      (5, golden :3653/:3654/:3659/:3663/:3664 -- both branches of the
//      AutoClean-Label block, see DESIGN NOTE below), CheckBySiteYieldAlarm
//      (2, golden :3861/:3927), CheckLowYieldAlarm (17, golden :4577/:4604/
//      :4623/:4653/:4656/:4676/:4679/:4727/:4730/:4742/:4809/:4812/:4832/
//      :4835/:4859/:4862 -- confined to the `IniConfig.bLowYieldAlarmSameNS`/
//      `IniConfig.bE53LowYieldAutoClean` config-flag branches, PLUS the
//      unconditional :4949 `fContactCT->SaveTotalYield("")` tail call),
//      CheckIntervalLowYieldAlarmBySite (1, golden :5566
//      `fContactCT->sgYield->Refresh()`).
//  (Y2) fShowBinSelect -- ⚠ STATUS UPDATE, AI(W906-FW-SBWB) 20260818: FULLY
//      DISSOLVED. forms/fShowBinSelect.h landed (FW-CTBS), the global became
//      a real instance (FW-YEnable), and the 8 labels this block lists as
//      missing landed 20260818 (SBWB wave) -- all 14 sites are ACTIVE now
//      (4 dissolved by FW-YEnable, the remaining 10 by SBWB). The text below
//      is kept as the historical record of WHY the gates existed; every
//      factual claim in it about absence is stale.
//      [HISTORICAL] (golden cShowBinSelect.h, TfShowBinSelect*) -- PARTIALLY
//      seamed elsewhere. ainarm9045.cpp:7514-7517 and csystem.cpp:4855-4862
//      both carry TU-local macro seams (`W7dK7_ShowBinSelect()` /
//      `W7C1_TfShowBinSelectSeam`), but BOTH cover only `UPH_StringGrid`
//      (ainarm9045.cpp's own SEAM (k7-S1)) -- neither exposes labArmDiff/
//      labSiteDiff/labLowYield/labTotalYield/labTotalYieldTotal/
//      lblSpeciallYield/lblSpeciallYieldTotal/IntervalByTotal/
//      ShowCategoryBin()/iLowYieldBinSelectContactCount, the members our 17
//      methods need. Confirmed absent otherwise: `grep -rn "fShowBinSelect"
//      --include=*.h forms/` -- 0 hits (no forms/fShowBinSelect.h exists at
//      all). Same "documented GAP, don't invent a third seam shape" posture
//      as (Y1); every touch is `#if 0`-gated with a pointer back here, 14
//      sites total, exact golden line numbers verified this wave against a
//      CRLF-normalized cp950-decoded copy (`sed -n` line-number cross-check,
//      20260818): CheckByPickerYieldAlarm (1, golden :4247 labArmDiff),
//      CheckBySiteByArmYieldAlarm (2, golden :4513/:4517 labSiteDiff),
//      CheckLowYieldAlarm (2, golden :4639/:4946 labLowYield -- caption
//      only, the underlying alarm logic these captions merely mirror is
//      untouched by the gate), CheckLowYieldAlarmByTotal (6, golden :4987
//      ShowCategoryBin + :5084/:5087/:5089/:5094/:5095 labTotalYield/
//      labTotalYieldTotal), ClearYieldCount (1, golden :5171
//      iLowYieldBinSelectContactCount), CheckIntervalLowYieldAlarmBySite (1,
//      golden :5515 ShowCategoryBin), CheckIntervalLowYieldAlarmByTotal (2,
//      golden :5649 ShowCategoryBin + :5670 IntervalByTotal),
//      CheckLowYieldAlarmSpecial (4, golden :5791/:5792/:5797/:5798
//      lblSpeciallYield/lblSpeciallYieldTotal, twice each -- populated and
//      zeroed arms). Two sites (CheckLowYieldAlarmByTotal's `str.sprintf(...)`
//      and CheckLowYieldAlarmSpecial's) gate the STRING-FORMAT line together
//      with its Caption consumer rather than leaving `str` write-only --
//      each has its own `(void)str;` right after the gated block, matching
//      CheckBySiteYieldAlarm's `(void)dYield;` precedent for the same
//      "gate made a real local go quiet" situation.
//  (Y3) fLotInfo -- ⚠ STATUS UPDATE, AI(W906-FW-Y3) 20260819: DISSOLVED.
//      The 5 members landed in forms/fLotInfo.h (allocated in its ctor);
//      both CalculateSiteYield arms are ACTIVE now. Text below is the
//      historical record; its absence claims are stale.
//      [HISTORICAL] fLotInfo->Label17/18/21->Caption + edtAutoCleanLowYield/
//      edtAutoCleanSiteYieldDiff->Text (CalculateSiteYield, golden
//      :3647-3664, the AutoClean-mode display block). forms/fLotInfo.h EXISTS
//      (unlike fContactCT/fShowBinSelect) but carries NONE of these five
//      members (`grep -n "Label17\|Label18\|Label21\|edtAutoCleanLowYield\|
//      edtAutoCleanSiteYieldDiff" forms/fLotInfo.h` -- 0 hits, 20260818).
//      Every RHS in that block ALSO calls the (Y1)-gapped
//      fContactCT->GetLowYield_AutoClean, so there is no independently
//      computable value to shadow-store the way fObserver.h's own
//      output-sink exception does (see that header's DESIGN NOTES on
//      TfObserverGrid) -- both the source and the sink are gapped, so the
//      whole 4-assignment block (both the `if` and `else` arms) is `#if 0`,
//      leaving only the ONE real, gate-free statement in that scope
//      (`iAutoClean_FailAlarmSiteYieldIntervalCount=0;`, `else` arm) ACTIVE.
//
//  DESIGN NOTE -- fMain->CleanOut/ShowTestHeadComp are NOT gated
//  --------------------------------------------------------------------------
//  Unlike fContactCT/fShowBinSelect/fLotInfo's missing members above,
//  forms/fMain.h ALREADY declares `virtual void ShowTestHeadComp(bool)`
//  (line 156) and `virtual void CleanOut(AnsiString)` (line 168) as real,
//  already-ACTIVE facade methods (grepped 20260818). CheckLowYieldAlarmSpecial's
//  `fMain->CleanOut("CheckLowYieldAlarmSpecial")` (golden :5763) is translated
//  as a genuine, ungated call.
//
//  DESIGN NOTE -- SlidingWindowYield fields: NOT a facade gap
//  --------------------------------------------------------------------------
//  The task brief flagged `Prod.bSlidingWindowYield`/`Prod.iSlidingWindowSize`
//  and `TestIF_File`'s matching pair as a SUSPECTED port gap needing a
//  PORT-ONLY shadow field. Checked this wave (`grep -n
//  "bSlidingWindowYield\|iSlidingWindowSize" cprod.h`): BOTH already exist,
//  in BOTH structs -- PROD_INFO_ST (cprod.h:645-646, backs the global `Prod`)
//  and SYSTEM_TEST_IF (cprod.h:1810-1811, backs `TestIF`/`TestIF_File`/
//  `TestIF_NET`) -- landed by an earlier, unrelated wave. No gate, no
//  PORT-ONLY field; CalculateSiteYield/CheckLowYieldAlarm read
//  `Prod.bSlidingWindowYield`/`Prod.iSlidingWindowSize` directly, verbatim.
//
//  DESIGN NOTE -- the one widget member (cbbClosedSiteBin)
//  --------------------------------------------------------------------------
//  SetClosedSiteBin (golden :5972-5980) is the ONLY one of these 17 methods
//  that touches a real golden widget: `TComboBox *cbbClosedSiteBin;` (golden
//  uYieldMonitoring.h:371). vclcompat::TComboBox (reused as-is, same as every
//  other translated form) already has `->Clear()` and `->Items->Add(...)` --
//  exactly what golden's body needs, so this facade adds nothing beyond
//  heap-allocating one instance via NSDMI.
// =============================================================================
#ifndef FORMS_FYIELDMONITORING_H
#define FORMS_FYIELDMONITORING_H

#include "vclcompat/Controls.h"   // vclcompat::TComboBox (cbbClosedSiteBin)
#include "MachineType.h"          // MAX_SOCKET_ROW/MAX_SOCKET_COL (array sizing, matches golden's own 4/8 literals)
// AI(W906-FW-YM-W14) 20260826: 下面的 handler 宣告帶 TMouseButton/TShiftState。
#include "vclcompat/ShiftState.h"

using vclcompat::TComboBox;

class TfYieldMonitoring
{
public:
    // -- ctor-populated widget member (NSDMI bootstrap, see banner above) ---
    TComboBox *cbbClosedSiteBin = new TComboBox();   // golden uYieldMonitoring.h:371 -- SetClosedSiteBin

    // -- range-check bounds (golden uYieldMonitoring.h:555-556) --------------
    // Real values are populated by golden's DoIniDataToForm/FormCreate (Wave
    // C, out of scope) -- defaulted to 0 here, same as every other member
    // this wave adds without its golden ctor in scope. tests/
    // test_yieldmon_core.cpp sets these explicitly before calling
    // CheckSettingNo (same "direct-write-globals-then-call" idiom as
    // tests/test_ga1_cprod.cpp).
    int iMinCount = 0;   // golden uYieldMonitoring.h:555
    int iMaxCount = 0;   // golden uYieldMonitoring.h:556

    // -- yield-engine data members (golden uYieldMonitoring.h:581-607) -------
    int iAutoClean_FailAlarmSiteYieldIntervalCount = 0;   // golden :581 -- CalculateSiteYield/CheckBySiteYieldAlarm
    int iLowYieldContactCount = 0;                        // golden :582 -- CheckLowYieldAlarm/ClearYieldCount
    int iFailAlarmSiteYieldIntervalCount = 0;             // golden :583 -- CheckBySiteYieldAlarm/ClearYieldCount
    int iPickerYieldIntervalCount = 0;                    // golden :584 -- CheckByPickerYieldAlarm/ClearYieldCount
    int iFailAlarmSiteMaxYieldIntervalCount = 0;          // golden :585 -- CheckBySiteByArmYieldAlarm/ClearYieldCount

    bool   bShowSiteYield[32] = {};                                       // golden :586
    double dSiteYield[MAX_SOCKET_ROW][MAX_SOCKET_COL] = {};               // golden :587 (literal 4/8 in golden)
    double dPickerYield[2][2][MAX_SOCKET_COL] = {};                       // golden :588
    double dMaxPickerYield = 0.0;                                          // golden :589
    int    iAlarmSiteYieldCmpCnt[MAX_SOCKET_ROW][MAX_SOCKET_COL] = {};    // golden :590
    double dIndexZ1Yield = 0.0, dIndexZ2Yield = 0.0;                       // golden :591

    // golden :592 -- every Check* method's own first line is `if(bFirstCount)
    // { <static-counter>=0; return; }`, i.e. bFirstCount==true SKIPS the
    // method's real body. Real init happens in golden's FormCreate (Wave C,
    // out of scope); defaulted false here (methods run for real by default)
    // rather than true (which would make every Check* method a no-op by
    // default) -- tests/test_yieldmon_core.cpp sets it explicitly either way
    // per-case.
    bool bFirstCount = false;   // golden :592

    int  iLowYieldByTotalContactCount = 0;   // golden :603 -- CheckLowYieldAlarmByTotal/ClearYieldCount
    bool bGetGPIBAutoSiteOff = false;        // golden :607 -- CanAutoCloseSite
    bool bHasCloseSite = false;              // golden :594 -- DoAutoCloseSite. NOTE: golden's
                                             // uYieldMonitoring.cpp:72 ALSO has a file-scope
                                             // `bool bHasCloseSite;` -- dead there (class scope
                                             // wins inside every member function); mirrored in
                                             // the port TU for structural parity.

    // -- Sliding Window Yield ring buffer (golden :609-618, verbatim) --------
    struct TSWRing
    {
        unsigned char bPass[2000];
        int  iHead;
        int  iCount;
        int  iPassCT;
        unsigned long iLastBySiteTotal;
        unsigned long iLastBySitePass;
    };
    // golden leaves this array uninitialized (POD, no ctor); zero-initialized
    // here via NSDMI `= {}` -- same "no golden ctor in scope -> NSDMI zero"
    // convention as every other member above (and forms/fObserver.h Wave 2's
    // dTempHistroy/fRecordIndexTime precedent), not a behaviour change (every
    // SW[][][]  cell is written before being read -- SWRingReset/SWRingPush
    // are the only writers, both this wave, both unconditional loops over the
    // full array from ClearYieldCount).
    TSWRing SW[2][MAX_SOCKET_ROW][MAX_SOCKET_COL] = {};   // golden :618

    // -- Wave A translated methods (bodies: uYieldMonitoring.cpp, this wave) -
    virtual void CalculateSiteYield();                              // golden :3639-3808
    virtual void CheckBySiteYieldAlarm();                            // golden :3809-4032
    virtual void CheckByPickerYieldAlarm();                          // golden :4033-4255
    virtual void ClearAutoSiteOffStatus();                           // golden :4256-4267
    virtual void CheckBySiteByArmYieldAlarm();                       // golden :4268-4521
    virtual void CheckLowYieldAlarm();                               // golden :4522-4956
    virtual void CheckLowYieldAlarmByTotal();                        // golden :4957-5102
    virtual void SWRingReset(int arm, int row, int col);             // golden :5105-5113
    virtual void SWRingPush(int arm, int row, int col, bool bIsPass, int N);  // golden :5115-5129
    virtual void CheckLowYieldAlarm_SW();                            // golden :5131-5158
    virtual void ClearYieldCount();                                  // golden :5161-5200
    virtual bool CanAutoCloseSite(int iAllSiteOn);                   // golden :5300-5334
    virtual void CheckIntervalLowYieldAlarmBySite();                 // golden :5490-5592
    virtual void CheckIntervalLowYieldAlarmByTotal();                // golden :5627-5701
    virtual void CheckLowYieldAlarmSpecial();                        // golden :5703-5805

    // -- DEVIATION: public, golden private (see banner) ----------------------
    virtual void CheckSettingNo();                                   // golden :3220-3231

    virtual void SetClosedSiteBin();                                 // golden :5972-5980

    // AI(W906-FW-Q3) 20260818: REAL BODIES, user-approved queue item 3
    // (behaviour-change wave, own commit + double gate). Signature restored
    // to golden's `int iAllSiteOn` (0=low-yield close, 1=all-site on,
    // 2=RT close -- golden's own :5336-5339 comment); the 26 arm-variant
    // call sites already carry golden's literal args verbatim (25x `0`,
    // 1x `1`, 1x `false` -- golden itself has that same `false` implicit
    // conversion), so NO call-site edits. The former documented no-op
    // (FW-YMSwap) is retired.
    virtual void DoAutoCloseSite(int iAllSiteOn);                    // golden :5340-5401
    virtual void DoRTAutoSocketOff();                                // golden :5202-5299

    // PORT-ONLY: no user-declared ctor/dtor this wave (see banner "NO
    // CONSTRUCTOR THIS WAVE" above) -- NSDMI on every member does the work a
    // ctor would, and no method needs virtual-dispatch-through-delete
    // (nothing in this wave or its test ever deletes through a
    // TfYieldMonitoring* base pointer, only ever a concrete-typed local).

    // ======================================================================
    // AI(W906-FW-YM-W14) 20260826: 19 支 UI 事件處理器（原排 21，兩支退出見 .cpp） + 它們解參照的 25 個
    // widget。widget 擁有者用 tools/wavescan/owner_report.py 對 golden header
    // 逐個查過（本波回報「屬於別的物件: 0 個」）——不是從 `x->y` 猜的。
    // 15 支帶 golden 完整的 `TMouseButton Button, TShiftState Shift` 簽章，
    // 靠 vclcompat/ShiftState.h（commit f184093）才成立；見該檔檔頭的裁決。
    // 依本 header :40-54 的慣例全部用 NSDMI，不寫建構子。
    // ======================================================================

    TButton        *btnApply                       = new TButton();   // golden uYieldMonitoring.h:19
    TCheckBox      *cbIntervalLowYieldBySite_FT    = new TCheckBox();   // golden uYieldMonitoring.h:135
    TCheckBox      *cbIntervalLowYieldBySite_RT    = new TCheckBox();   // golden uYieldMonitoring.h:148
    TCheckBox      *cbIntervalLowYieldByTotal_FT   = new TCheckBox();   // golden uYieldMonitoring.h:141
    TCheckBox      *cbIntervalLowYieldByTotal_RT   = new TCheckBox();   // golden uYieldMonitoring.h:154
    TCheckBox      *cbLowYieldByTotal_FT           = new TCheckBox();   // golden uYieldMonitoring.h:197
    TCheckBox      *cbLowYieldByTotal_RT           = new TCheckBox();   // golden uYieldMonitoring.h:259
    TCheckBox      *cbLowYield_FT                  = new TCheckBox();   // golden uYieldMonitoring.h:180
    TCheckBox      *cbLowYield_RT                  = new TCheckBox();   // golden uYieldMonitoring.h:245
    TCheckBox      *cbSiteYieldCmp_FT              = new TCheckBox();   // golden uYieldMonitoring.h:193
    TCheckBox      *cbSiteYieldCmp_RT              = new TCheckBox();   // golden uYieldMonitoring.h:254
    TCheckBox      *cbSiteYieldDifferent_FT        = new TCheckBox();   // golden uYieldMonitoring.h:186
    TCheckBox      *cbSiteYieldDifferent_RT        = new TCheckBox();   // golden uYieldMonitoring.h:251
    TEdit          *edLowYieldByTotalIg_FT         = new TEdit();   // golden uYieldMonitoring.h:199
    TEdit          *edLowYieldByTotalIg_RT         = new TEdit();   // golden uYieldMonitoring.h:262
    TEdit          *edSiteYieldCmpIg_FT            = new TEdit();   // golden uYieldMonitoring.h:191
    TEdit          *edSiteYieldCmpIg_RT            = new TEdit();   // golden uYieldMonitoring.h:257
    TRadioButton   *rbContsFailByHead_FTOff        = new TRadioButton();   // golden uYieldMonitoring.h:110
    TRadioButton   *rbContsFailByHead_FTOn         = new TRadioButton();   // golden uYieldMonitoring.h:109
    TRadioButton   *rbContsFailByHead_RTOff        = new TRadioButton();   // golden uYieldMonitoring.h:236
    TRadioButton   *rbContsFailByHead_RTOn         = new TRadioButton();   // golden uYieldMonitoring.h:235
    TRadioButton   *rbContsFailBySocket_FTOff      = new TRadioButton();   // golden uYieldMonitoring.h:105
    TRadioButton   *rbContsFailBySocket_FTOn       = new TRadioButton();   // golden uYieldMonitoring.h:104
    TRadioButton   *rbContsFailBySocket_RTOff      = new TRadioButton();   // golden uYieldMonitoring.h:231
    TRadioButton   *rbContsFailBySocket_RTOn       = new TRadioButton();   // golden uYieldMonitoring.h:230

    void cbLowYieldByTotal_FTMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   // golden :5442-5456
    void rbContsFailBySocket_FTOnMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   // golden :3301-3310
    void rbContsFailByHead_FTOnMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   // golden :3312-3321
    void rbContsFailBySocket_RTOnMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   // golden :3323-3332
    void rbContsFailByHead_RTOnMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   // golden :3334-3343
    void cbIntervalLowYieldBySite_FTMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   // golden :5431-5440
    void cbSiteYieldCmp_RTMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   // golden :5458-5467
    void cbIntervalLowYieldBySite_RTMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   // golden :5479-5488
    void cbIntervalLowYieldByTotal_FTMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   // golden :5605-5614
    void cbIntervalLowYieldByTotal_RTMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   // golden :5616-5625
    void cbLowYield_FTMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   // golden :3281-3289
    void cbSiteYieldDifferent_FTMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   // golden :3291-3299
    void cbLowYield_RTMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   // golden :3345-3353
    void cbSiteYieldDifferent_RTMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   // golden :3355-3363
    void cbLowYieldByTotal_RTMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   // golden :5469-5477
    void edFailYieldRate_ARTFTFileKeyPress(TObject *Sender, char &Key);   // golden :3554-3559
    void rgPiggyBack_FTClick(TObject *Sender);   // golden :3147-3150
    void rgQARunModeClick(TObject *Sender);   // golden :3233-3236
    void rgBinAlarmByClick(TObject *Sender);
    // AI(W906-YM-READFILE) 20260923: golden uYieldMonitoring.cpp:895-1700
    // (806 L).  The tree's largest single ini reader -- 253 ReadIniData /
    // CheckAndReadIniData over Tester.Data.  Un-gates RunStartMode.cpp's
    // SAFETY-GATE(W906-P10-YIELDREAD), whose stated condition was exactly
    // "wait until this has a real body".
    void ReadFile();

    // AI(W906-YM-READFILE) 20260923: two range limits ReadFile() passes to
    // CheckRange (golden :3271 and friends).  They are TfYieldMonitoring form
    // members (golden uYieldMonitoring.h:551-552) set once in golden's ctor
    // (golden uYieldMonitoring.cpp:292-293 `iMinYield=0; iMaxYield=100;`).
    // Declared here WITH those values rather than gating the lines that use
    // them: those lines each carry a real ReadIniData, so gating would have
    // dropped a genuine ini read to avoid declaring two ints.
    int iMinYield = 0;            // golden uYieldMonitoring.h:551 / ctor :292
    int iMaxYield = 100;          // golden uYieldMonitoring.h:552 / ctor :293   // golden :3276-3279
};

// AI(W906-FW-YMSwap) 20260818: the integration call was made (user-approved
// queue): the live global is backed by THIS facade. TfYieldMonitoring_2x4_16
// retired from aHotPlateSubstrate.h / ainarm9045_2x4_16_shims.{h,cpp}; the
// global's definition homecame to uYieldMonitoring.cpp (golden
// uYieldMonitoring.h declares `extern PACKAGE TfYieldMonitoring
// *fYieldMonitoring;`). Static-init construction is trivially safe: this
// class has NO user ctor (NSDMI only) and touches no config layer.
extern TfYieldMonitoring *fYieldMonitoring;      // golden uYieldMonitoring.h (extern PACKAGE)

#endif // FORMS_FYIELDMONITORING_H
