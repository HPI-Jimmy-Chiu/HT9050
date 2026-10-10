// =============================================================================
//  forms/fStartCondition.h  --  non-VCL facade for golden's TfStartCondition
//  (cStartCondition.h/.cpp)
//
//  AI(W906-FW3-StartCond-WA) 20260819: new file, batch-5 dual-form display-side
//  wave (cStartCondition + HandlerSys). Golden ref:
//  HT9011UC_Code_V3.33.906.0_20260618/cStartCondition.h (452 lines) + .cpp
//  (1,716 lines, 35 methods; cp950 -- decoded with `python3 -c
//  "open(...,'rb').read().decode('cp950')"`, 0 U+FFFD, this wave) + .dfm.
//  No prior forms/fStartCondition.h or any same-named stand-in existed before
//  this wave (grepped 20260819: `grep -rn "StartCondition" HT9011UC_Cpp_V3.33.906.0`
//  -- only tools/dfm2rc/* generator artifacts and this wave's own new files;
//  no ODR-risk sibling, unlike the tree's two-TMyKitSuck precedent).
//
//  ROLE
//  ----
//  TfStartCondition is golden's "Start Condition / Life Time" dialog: FT/RT
//  counter-clear checkboxes, contact-count-alarm grids (per-head and
//  per-socket), Socket-ID life-time tracking, picker pick-count life-time
//  counters, and a "Pre-Alarm Cylinder" tab (on/off count + on/off time
//  tracking per pneumatic cylinder, added 20230516). This wave's mandate is
//  BATCH 5, scope = read-only current-value DISPLAY side only; every
//  editing/persist path is queued (see METHOD CLASSIFICATION below).
//
//  METHOD CLASSIFICATION (all 35 golden methods; the mandate's 4-bucket taxonomy)
//  --------------------------------------------------------------------------
//  (a) DISPLAY/READ-ONLY -- TRANSLATED this wave (8 methods, bodies in
//      cStartCondition.cpp):
//        TfStartCondition() ctor  golden :40-104  (widget-array wiring only;
//                                   GDI setup dropped, see DEVIATION)
//        FormShow                golden :106-536  (431 lines -- the form's
//                                   own display refresh; two GOLDEN ODDITY
//                                   hidden writes to LastSet.iStartMode
//                                   #if 0-gated, see GATE (SC1)/(SC2); one
//                                   cross-form GATE, see GATE (SC3))
//        DoIniDataToForm         golden :1009-1027 (pure LastSet -> widget
//                                   Text/Caption refresh, zero I/O)
//        pgLifeTimeChange         golden :1029-1047 (pure TestIF_File ->
//                                   editContactCountAlarm refresh)
//        GetCylinderIndexByName   golden :1479-1487 (pure Cylinder[] lookup,
//                                   no side effect)
//        UpdateCylinderScreen     golden :1489-1589 (display refresh of the
//                                   Pre-Alarm Cylinder grid from live
//                                   Cylinder[] objects; ONE embedded write --
//                                   see GATE (SC4) GOLDEN ODDITY)
//        LoadCylinderLife         golden :1591-1653 (loads Cylinder[]
//                                   on/off-count + alarm-threshold + on/off-
//                                   time-history from asMachineLifePath --
//                                   see DISCLOSED note, same class as fSpeed.h's
//                                   CheckAndReadIniData note)
//        strngrdCylinderViewDrawCell golden :1418-1450 (custom-draw colour
//                                   decision; GATEd, see GATE (SC5) -- same
//                                   "no TCanvas anywhere in vclcompat" class
//                                   as forms/fContactCT.h's GATE (C1))
//
//  (b) WRITE PATH -- QUEUED, NOT TRANSLATED, NOT EVEN DECLARED this wave (per
//      forms/fTemperFrom.h's "not even a stub declared" precedent) -- 25
//      methods:
//        FormClose (:538-588 -- calls sbSaveClick, writes LastSet.iStartMode,
//          re-reads+writes TestIF_File.iContactAlarmCount[] via
//          CheckAndReadIniData), sbSameAsHead1Click (:590-605, copies a grid
//          cell -- stages an edit, no persist of its own but the field it
//          stages is a Save-consumed cell), sbClearCountClick (:607-625),
//          sbSaveClick (:627-658, ~90-style WriteIniData + WriteLastDataFile),
//          sgContactCountSelectCell (:660-668, opens QwertyKey + writes cell),
//          spbExitClick (:670-689, writes LastSet.bCTClear[][] +
//          WriteLastDataFile), sbHeadCondition1ClearClick (:691-758, zeroes
//          grids/IniConfig.iVibrator*), ReadWriteStartCondition (:760-924,
//          bRead-parameterized -- NOT translated this wave at all, unlike
//          fSpeed's ReadWriteFile: its READ arm's own hydration order matters
//          for a GOLDEN ODDITY the RogerYang 20260203 comment at :888
//          documents -- "修正formshow裡的fSetup->ReadFile()又調用
//          ReadWriteFile()會造成銦片計數異常" -- i.e. golden's own bugfix
//          history shows this function is sensitive to call ORDER against
//          FormShow's fSetup->ReadFile() [itself GATEd, see (SC3)], so a
//          partial read-arm-only translation here would misrepresent a
//          function golden's own authors have already had to patch race
//          conditions around; queued whole, not split, pending a future wave
//          that also resolves (SC3)), sbHeadCondition1SaveClick (:926-974),
//          sgHeadCondition1SelectCell (:976-1007, opens QwertyKey + writes
//          cell, has an AMD_M reset-to-zero confirm dialog),
//          editContactCountAlarmMouseDown / PanelAaDblClick /
//          edtInOutArmPickerAlmCntClick / edtInArmAClick /
//          edtSetXYOffsetLimitClick / edtSetZOffsetLimitClick (6 QwertyKey-
//          opener MouseDown/Click handlers, same "edit gesture" bucket as
//          fSpeed.h's ~24 MouseDown handlers), cbStartModeOnlyFTClick
//          (:1061-1078, writes LastSet.iStartMode), ClearPickerCount
//          (:1086-1130) + btnInArmAClick/btnOutArmAClick/btnArm2AaClick/
//          btnArm1AaClick (4 thin callers) + WritePickerCount (:1160-1203,
//          WriteIniData), btnClearAaClick (:1216-1232, zeroes
//          LastSet.iSocketContactCount[][]), btnSetOffsetLimitClick
//          (:1234-1249, WriteIniData to Security_new.def -- SAFETY-ADJACENT,
//          see (c) below), SocketIDLog (:1261-1318, fopen/fputs a CSV file --
//          a WRITE despite the "Log" name, same "Show/Read/Close 前綴會騙人"
//          class this tree already tracks, just spelled "Log" instead),
//          mniResetOnOffCountClick (:1321-1416, resets/edits cylinder
//          counts+alarm thresholds, calls SaveCylinderLife), SaveCylinderLife
//          (:1655-1714, ~15 WriteIniData calls per enabled cylinder).
//
//  (c) SAFETY-ADJACENT -- subset of (b), called out separately per the
//      campaign's own bucket, NOT translated this wave (queued, same as (b)):
//        btnSetOffsetLimitClick writes `Input Limit`/`funLimit[0..3,13,14]`
//        into `Security_new.def` -- these are the motion soft-limit XY/Z
//        offset bounds (funLimit indices 0-3/13-14 -- golden's own array,
//        not re-derived here). A wrong value here changes how far off-taught-
//        position a motor is allowed to drift before a soft-limit alarm
//        fires. Not a direct interlock bypass, but adjacent enough to flag
//        explicitly rather than silently bucket with the ~90-line generic
//        WriteIniData calls in sbSaveClick/SaveSystemSet.
//
//  (d) GOLDEN-DEAD or STRUCTURALLY GATED -- none found distinct from the (a)/
//      (b) GATE notes below; every method has a live, reachable body in
//      golden with no `//`-commented-out sibling fragment inside it worth a
//      separate call-out (contrast forms/fSpeed.h's two golden-commented
//      fragments -- none exist in this file).
//
//  GATE REGISTER
//  --------------------------------------------------------------------------
//  (SC1) FormShow, golden :142-147 `if(IniConfig.bShowFTandRTButtonCanClick==
//      true && CUSTOMER_CODE!=CC_SCC){ LastSet.iStartMode=2;
//      gbStartMode->Visible=false; }` -- the `LastSet.iStartMode=2;` line is
//      a WRITE hidden inside a function named "Show" (this tree's own
//      tracked "Show/Read/Close 前綴會騙人" pattern, four prior instances
//      already on record). `#if 0`-gated; `gbStartMode->Visible=false;`
//      (a pure display action, no data-model write) is kept ACTIVE.
//  (SC2) FormShow, golden :169-172 `if(cbStartModeOnlyFT->Checked==true){
//      LastSet.iStartMode=1; }` -- same class as (SC1): a checkbox-read
//      driving a hidden LastSet write. `#if 0`-gated.
//      CONSEQUENCE OF (SC1)+(SC2) BOTH GATED: golden :174-179's
//      `if(LastSet.iStartMode==0) rbStartMode1->Checked=true; else if(==1)
//      rbStartMode2->Checked=true; else rbStartMode3->Checked=true;` is kept
//      ACTIVE (it is a pure display decision) but now reads whatever
//      LastSet.iStartMode held BEFORE this FormShow call rather than the
//      value golden would have forced moments earlier -- i.e. in the narrow
//      case where `bShowFTandRTButtonCanClick`+non-SCC or
//      `cbStartModeOnlyFT->Checked` is true, this translated FormShow can
//      display a start-mode radio button that golden would not have shown.
//      Disclosed, not silently absorbed; resolving it is a write-path-wave
//      decision (does the display side get to special-case "read but don't
//      persist this forced value", or does it wait for the real write path).
//  (SC3) RESOLVED 20260922 -- TfSetup::ReadFile() now exists (cSetUp.cpp,
//      golden cSetUp.cpp:2191-3020) and the call at cStartCondition.cpp:167
//      is un-gated.  The DISCLOSED CONSEQUENCE below no longer applies.
//      Original note kept for the reasoning:
//  (SC3) FormShow, golden :167 `fSetup->ReadFile();` -- forms/fSetup.h
//      (landed W7-L2, 20260803) is a ONE-FIELD facade (`bool fShow;` only,
//      by design -- see its own file-head "GOLDEN ckernel.cpp CENSUS"
//      measuring exactly one call site tree-wide before this wave). It has
//      NO `ReadFile()` method. `#if 0`-gated. DISCLOSED CONSEQUENCE: golden's
//      own call graph has fSetup->ReadFile() refresh the site-map-derived
//      `FTestSuck.iShtRow`/`iShtCol` (consumed later in this SAME FormShow,
//      golden :255-288, ACTIVE in this wave's translation) before those are
//      read; with the gate, this translated FormShow displays the Pre-Alarm
//      per-head contact grid using whatever `FTestSuck.iShtRow`/`iShtCol`
//      already held in memory at call time, not a freshly-reloaded value.
//  (SC4) UpdateCylinderScreen, golden :1565-1572 `if(Cylinder[i].iOnOffCount<0)
//      {...NewRecordProcess(...); Cylinder[i].iOnOffCount=0; SaveCylinderLife();}`
//      -- a self-healing WRITE (clamp an overflowed counter back to zero and
//      persist it) hidden inside a function named "UpdateScreen". Same class
//      as (SC1)/(SC2)/SocketIDLog: a benign-sounding name hiding a real
//      write. Per the campaign's "讀寫合體函式只翻讀臂＋寫臂 #if 0" rule
//      (the closest applicable precedent, even though this isn't a
//      bRead-parameterized function): `#if 0`-gated the
//      NewRecordProcess+assignment+SaveCylinderLife trio; the surrounding
//      `sCylinderData[eCylOnOffCnt][iRow]=Cylinder[i].iOnOffCount;` display
//      line is kept ACTIVE and will simply display a negative count verbatim
//      in the (rare, overflow-only) case this guard would have caught.
//  (SC5) strngrdCylinderViewDrawCell -- vclcompat has NO `class TCanvas`
//      anywhere (grepped this wave, 0 hits, same finding forms/fContactCT.h's
//      GATE (C1) and forms/fLotInfo.h/EJ1N/MyOmronPanel.h/
//      VacuumUnit/MyVacuumPanel.h independently record for their own GDI
//      call sites). The BrushColor/FontColor decision this method computes
//      (compare current count/time against its alarm threshold, pick
//      yellow/white) has NO independent consumer besides feeding
//      `MyDrawText(pCanvasCyl, ...)` -- no shadow array stores "is this cell
//      over-threshold" for any other reader. Per forms/fYieldMonitoring.h's
//      GATE (Y3) "no independently computable value to shadow-store"
//      precedent (also cited by fContactCT.h's GATE (C1)), the ENTIRE method
//      body is `#if 0`-gated rather than half-translating a value nothing
//      else reads. Declared with a trimmed (ACol, ARow) signature (Sender/
//      Rect/State dropped, same DEVIATION as fContactCT.h's
//      sgYieldDrawCell) so it is ready the moment a TCanvas lands.
//
//  DEVIATION -- GDI members dropped entirely (PtrDCCyl/pCanvasCyl)
//  --------------------------------------------------------------------------
//  Golden ctor (:102-103) does `PtrDCCyl=GetDC(strngrdCylinderView->Handle);
//  pCanvasCyl=new TCanvas;` purely to feed strngrdCylinderViewDrawCell's
//  MyDrawText calls -- which are entirely GATE (SC5) content. Same posture as
//  forms/fContactCT.h's identical hDC/pCanvas DEVIATION: not declared here at
//  all rather than invented as dead members.
//
//  DEVIATION -- widget list is NOT golden's complete __published set
//  --------------------------------------------------------------------------
//  Unlike forms/fSpeed.h's "declare all 330, matching future-proofing"
//  precedent, this header declares ONLY the ~130 widgets this wave's 8
//  translated (a) methods actually touch (the socket-ID grid's 32
//  TEdit/32 TPanel/32 TButton arrays, the three per-head TStringGrid trio,
//  the FT/RT counter-clear checkboxes, the vibration/picker/offset-limit
//  edits, and the Pre-Alarm Cylinder tab). Reason for the deviation: golden's
//  remaining ~120 widgets are touched ONLY by bucket (b)/(c) methods (25 of
//  35, a much larger untranslated fraction than fSpeed's 47/57), and three of
//  golden's widget TYPES used only by those methods --
//  `TPopupMenu`/`TMenuItem` (mniResetOnOffCountClick,
//  strngrdCylinderViewSelectCell's context-menu wiring) and `TScrollBox`
//  (tsPickerLifeTime's ScrollBox1, a pure layout container no method reads
//  back) -- have ZERO port anywhere in vclcompat (grepped this wave:
//  `grep -rn "class TPopupMenu\|class TMenuItem\|class TScrollBox"` over the
//  whole tree, 0 hits). Inventing three new facade-only types purely so an
//  unused declaration exists would be manufacturing UI infrastructure with
//  no behaviour behind it -- the same "don't build UI infra" restraint
//  forms/fSpeed.h itself invokes for its own DoSetRPDefault GATE (S4).
//  Reported here, not silently shrunk: a future wave translating
//  mniResetOnOffCountClick/strngrdCylinderViewSelectCell (bucket (b)/(b))
//  will need to extend this header, matching fSpeed.h's own acknowledged
//  future-re-extension cost.
//
//  DESIGN NOTE -- TfStartConditionGrid (1 new local widget type)
//  --------------------------------------------------------------------------
//  golden's sgContactCount ALSO needs ->Width= (FormShow :190/:218, dynamic
//  column-count-driven resize) and strngrdCylinderView ALSO needs
//  ->ColWidths[i]= (UpdateCylinderScreen's one-time header setup, golden
//  :1505-1517), ->Height= (LoadCylinderLife's one-time RowCount-driven
//  resize, golden :1650) and ->Refresh() (end of UpdateCylinderScreen,
//  golden :1588). Width/Height are COSMETIC LAYOUT ONLY (nothing in this
//  wave's 8 translated methods reads either back) and stay LOCAL to this
//  class (this form is the only one of the 5 TStringGrid-extension facades
//  that needs plain geometry ints under these names); Refresh() is a
//  documented GATE (SC5) sibling no-op and also stays local (StartCondition/
//  ContactCT/Observer each carry their own near-identical GDI Refresh()
//  no-op -- a real but SEPARATE duplication from the ColWidths one below,
//  evaluated and deliberately left alone this wave, see file-tail note).
//
//  AI(W906-VclGrid-1) 20260820: ->ColWidths[i]= used to be this class's own
//  FIXED-SIZE `int ColWidths[16]` array (16 >= eCylPrAlarmItemTotal(13); only
//  indices 0..12 were ever assigned) -- a DIFFERENTLY-SHAPED, independent
//  implementation of the same property name that forms/fObserver.h's
//  TfObserverGrid and forms/fConfiguration.h's TfConfigurationGrid ALSO each
//  carried (as an auto-growing proxy, not a fixed array). Now that the
//  proxy shape lives on the shared vclcompat::TStringGrid base (see that
//  header's own SCOPE banner), keeping a local fixed-array `ColWidths` here
//  too would SHADOW the inherited member -- name-identical, type-different,
//  silently picking whichever one a given expression happens to bind to.
//  Removed in favour of the inherited proxy: verified behaviour-preserving
//  by reading every `strngrdCylinderView->ColWidths[...]` site in
//  cStartCondition.cpp (13 lines, :750-762, one WRITE per eCylPrAlarmItemTotal
//  enumerator, 0 reads) -- the base proxy's write-through semantics for
//  `ColWidths[idx]=value` are identical to a plain array's for every index
//  ever touched here.
//
//  HYDRATION (dfm: HT9011UC_Code_V3.33.906.0_20260618/cStartCondition.dfm,
//  this wave -- see project's own "TStringGrid default 5x5 already broke
//  twice" gotcha)
//  --------------------------------------------------------------------------
//    sgContactCount       : ColCount=3, RowCount=3   (dfm :645-660; FormShow
//                            overwrites ColCount to 4 when
//                            CosFunction.bContactAlmNeedOneCycle, exactly as
//                            golden does; RowCount stays 3 always -- header +
//                            2 heads)
//    sgHeadCondition1/2/3 : ColCount=4, RowCount=17  (dfm :688-821; header +
//                            16 Arm*Head data rows, i<2*j<8)
//    sgSocketCount        : ColCount=4, RowCount=33  (dfm :2748-2763; header
//                            + 4*8=32 socket data rows)
//    strngrdCylinderView  : ColCount=4, RowCount=10  (dfm :4031-4046 design-
//                            time placeholder -- LoadCylinderLife's `bfirst`
//                            block overwrites ColCount to
//                            eCylPrAlarmItemTotal=13 and RowCount to
//                            iRow+1 at runtime, exactly as golden does; this
//                            wave's translated LoadCylinderLife performs that
//                            same resize, so the dfm's design-time 4x10 is
//                            only the pre-first-load transient)
//
//  INTEGRATION-PENDING -- NO `extern TfStartCondition *fStartCondition;` here
//  --------------------------------------------------------------------------
//  Same posture as forms/fSpeed.h/forms/fTemperFrom.h: no consumer of a
//  `fStartCondition` global exists anywhere else in this tree yet (golden's
//  own LoadCylinderLife :1649-1651 self-references via
//  `fStartCondition->strngrdCylinderView` even from INSIDE
//  TfStartCondition's own method -- translated here using the plain
//  unqualified `strngrdCylinderView` member instead, a DEVIATION with no
//  behaviour change, avoiding the need to stand up a live singleton this
//  wave). Picking the live singleton wiring is a later integration wave's
//  call.
// =============================================================================
#ifndef FORMS_FSTARTCONDITION_H
#define FORMS_FSTARTCONDITION_H

#include "vclcompat/Controls.h"   // TEdit/TPanel/TLabel/TButton/TCheckBox/
                                  //   TRadioButton/TGroupBox/TPageControl/
                                  //   TTabSheet (brought to global scope by
                                  //   that header's own footer)
#include "vclcompat/StringGrid.h"

using vclcompat::TStringGrid;

// ---------------------------------------------------------------------------
//  TfStartConditionGrid -- see DESIGN NOTE above.
// ---------------------------------------------------------------------------
class TfStartConditionGrid : public vclcompat::TStringGrid
{
public:
    int Width  = 0;
    int Height = 0;
    // AI(W906-VclGrid-1) 20260820: ColWidths[] REMOVED from here -- now
    // inherited from vclcompat::TStringGrid (see DESIGN NOTE above for the
    // shadow-avoidance rationale and the behaviour-preservation check).

    explicit TfStartConditionGrid(int initialColCount = 5, int initialRowCount = 5)
        : vclcompat::TStringGrid(initialColCount, initialRowCount) {}

    // GATE (SC5) sibling: GDI no-op, see banner.
    void Refresh() {}
};

// ===========================================================================
//  TfStartCondition -- non-VCL facade (golden cStartCondition.h,
//  TfStartCondition : public TForm)
// ===========================================================================
class TfStartCondition
{
public:
    TfStartCondition();

    // -- golden :431 (public bool fShow) -------------------------------------
    bool fShow = false;

    // -- golden :443-444 (Pre-Alarm Cylinder selection state) ---------------
    // Only iSelRow/iSelCol are read by any (a)-bucket method (indirectly,
    // through GetCylinderIndexByName's caller in bucket (b)'s
    // mniResetOnOffCountClick); kept for structural fidelity even though no
    // (a) method assigns them this wave (strngrdCylinderViewSelectCell,
    // their only writer, is bucket (b), not translated).
    int iSelRow = 0;
    int iSelCol = 0;

    // -- golden :440 (Pre-Alarm Cylinder screen-text cache) ------------------
    // golden AnsiString sCylinderData[eCylPrAlarmItemTotal][MaxCylinderItem];
    // -- MachineType.h's eCylPrAlarmItemTotal(=13)/mycylin.h's
    // MaxCylinderItem(=321 since F9050-BD, was 295), both already ported.   [AI(W906-F9050-BD) 20261004: value updated]
    AnsiString sCylinderData[13][321];   //AI(W906-F9050-BD) 20261004: [295] -> [321]: golden's bound is MaxCylinderItem, now 321 (mycylin.h:42); this header does not include mycylin.h, so tests/test_flow9050_tray.cpp pins the literal with a static_assert

    // -----------------------------------------------------------------------
    //  Widget members touched by this wave's 8 translated (a) methods only
    //  -- see "widget list is NOT golden's complete __published set" DEVIATION
    //  above. Golden __published order preserved within each group.
    // -----------------------------------------------------------------------

    // -- FT/RT counter-clear checkboxes (golden :38-44, :59-65) --------------
    TCheckBox *cbLoadingCount_FT  = new TCheckBox();
    TCheckBox *cbTestCategory_FT  = new TCheckBox();
    TCheckBox *cbContactCount_FT  = new TCheckBox();
    TCheckBox *cbSortingCount_FT  = new TCheckBox();
    TCheckBox *cbScanner_FT       = new TCheckBox();
    TCheckBox *cbAlarmData_FT     = new TCheckBox();
    TCheckBox *cbTimeData_FT      = new TCheckBox();
    TCheckBox *cbLoadingCount_RT  = new TCheckBox();
    TCheckBox *cbTestCategory_RT  = new TCheckBox();
    TCheckBox *cbContactCount_RT  = new TCheckBox();
    TCheckBox *cbSortingCount_RT  = new TCheckBox();
    TCheckBox *cbScanner_RT       = new TCheckBox();
    TCheckBox *cbAlarmData_RT     = new TCheckBox();
    TCheckBox *cbTimeData_RT      = new TCheckBox();

    // -- Start Mode group (golden :25-28, :167) ------------------------------
    TGroupBox    *gbStartMode      = new TGroupBox();
    TRadioButton *rbStartMode1     = new TRadioButton();
    TRadioButton *rbStartMode2     = new TRadioButton();
    TRadioButton *rbStartMode3     = new TRadioButton();
    TCheckBox    *cbStartModeOnlyFT = new TCheckBox();

    // -- Contact Count grid (golden :49) -------------------------------------
    TfStartConditionGrid *sgContactCount = new TfStartConditionGrid(3, 3);

    // -- Picker pick-count life-time counters (golden :186-193, :211-218) ----
    TEdit *edtInArmA = new TEdit(), *edtInArmB = new TEdit(), *edtInArmC = new TEdit(), *edtInArmD = new TEdit();
    TEdit *edtInArmE = new TEdit(), *edtInArmF = new TEdit(), *edtInArmG = new TEdit(), *edtInArmH = new TEdit();
    TEdit *edtOutArmA = new TEdit(), *edtOutArmB = new TEdit(), *edtOutArmC = new TEdit(), *edtOutArmD = new TEdit();
    TEdit *edtOutArmE = new TEdit(), *edtOutArmF = new TEdit(), *edtOutArmG = new TEdit(), *edtOutArmH = new TEdit();
    TEdit *edtArm1Aa = new TEdit(), *edtArm1Ab = new TEdit(), *edtArm1Ac = new TEdit(), *edtArm1Ad = new TEdit();
    TEdit *edtArm1Ae = new TEdit(), *edtArm1Af = new TEdit(), *edtArm1Ag = new TEdit(), *edtArm1Ah = new TEdit();
    TEdit *edtArm1Ba = new TEdit(), *edtArm1Bb = new TEdit(), *edtArm1Bc = new TEdit(), *edtArm1Bd = new TEdit();
    TEdit *edtArm1Be = new TEdit(), *edtArm1Bf = new TEdit(), *edtArm1Bg = new TEdit(), *edtArm1Bh = new TEdit();
    TEdit *edtArm2Aa = new TEdit(), *edtArm2Ab = new TEdit(), *edtArm2Ac = new TEdit(), *edtArm2Ad = new TEdit();
    TEdit *edtArm2Ae = new TEdit(), *edtArm2Af = new TEdit(), *edtArm2Ag = new TEdit(), *edtArm2Ah = new TEdit();
    TEdit *edtArm2Ba = new TEdit(), *edtArm2Bb = new TEdit(), *edtArm2Bc = new TEdit(), *edtArm2Bd = new TEdit();
    TEdit *edtArm2Be = new TEdit(), *edtArm2Bf = new TEdit(), *edtArm2Bg = new TEdit(), *edtArm2Bh = new TEdit();
    TEdit *edtInOutArmPickerAlmCnt = new TEdit();

    // -- Life Time page control + per-head grids (golden :51-57) -------------
    TPageControl *pgLifeTime     = new TPageControl();
    TTabSheet    *tsCondition01  = new TTabSheet();
    TTabSheet    *tsCondition02  = new TTabSheet();
    TTabSheet    *tsCondition03  = new TTabSheet();
    TStringGrid  *sgHeadCondition1 = new TStringGrid(4, 17);
    TStringGrid  *sgHeadCondition2 = new TStringGrid(4, 17);
    TStringGrid  *sgHeadCondition3 = new TStringGrid(4, 17);

    // -- Socket ID / Socket Contact Count grid (golden :66-151) --------------
    TTabSheet *TabsSocketID = new TTabSheet();
    TLabel *labTestColA = new TLabel(), *labTestColB = new TLabel(), *labTestColC = new TLabel(), *labTestColD = new TLabel();
    TLabel *labTestColE = new TLabel(), *labTestColF = new TLabel(), *labTestColG = new TLabel(), *labTestColH = new TLabel();
    TLabel *labTestRowA = new TLabel(), *labTestRowB = new TLabel(), *labTestRowC = new TLabel(), *labTestRowD = new TLabel();
    TLabel *LabelID_RowA = new TLabel(), *LabelID_RowB = new TLabel(), *LabelID_RowC = new TLabel(), *LabelID_RowD = new TLabel();
    TLabel *LabelCnt_RowA = new TLabel(), *LabelCnt_RowB = new TLabel(), *LabelCnt_RowC = new TLabel(), *LabelCnt_RowD = new TLabel();
    TEdit *editSocketAa = new TEdit(), *editSocketAb = new TEdit(), *editSocketAc = new TEdit(), *editSocketAd = new TEdit();
    TEdit *editSocketAe = new TEdit(), *editSocketAf = new TEdit(), *editSocketAg = new TEdit(), *editSocketAh = new TEdit();
    TEdit *editSocketBa = new TEdit(), *editSocketBb = new TEdit(), *editSocketBc = new TEdit(), *editSocketBd = new TEdit();
    TEdit *editSocketBe = new TEdit(), *editSocketBf = new TEdit(), *editSocketBg = new TEdit(), *editSocketBh = new TEdit();
    TEdit *editSocketCa = new TEdit(), *editSocketCb = new TEdit(), *editSocketCc = new TEdit(), *editSocketCd = new TEdit();
    TEdit *editSocketCe = new TEdit(), *editSocketCf = new TEdit(), *editSocketCg = new TEdit(), *editSocketCh = new TEdit();
    TEdit *editSocketDa = new TEdit(), *editSocketDb = new TEdit(), *editSocketDc = new TEdit(), *editSocketDd = new TEdit();
    TEdit *editSocketDe = new TEdit(), *editSocketDf = new TEdit(), *editSocketDg = new TEdit(), *editSocketDh = new TEdit();
    TPanel *PanelAa = new TPanel(), *PanelAb = new TPanel(), *PanelAc = new TPanel(), *PanelAd = new TPanel();
    TPanel *PanelAe = new TPanel(), *PanelAf = new TPanel(), *PanelAg = new TPanel(), *PanelAh = new TPanel();
    TPanel *PanelBa = new TPanel(), *PanelBb = new TPanel(), *PanelBc = new TPanel(), *PanelBd = new TPanel();
    TPanel *PanelBe = new TPanel(), *PanelBf = new TPanel(), *PanelBg = new TPanel(), *PanelBh = new TPanel();
    TPanel *PanelCa = new TPanel(), *PanelCb = new TPanel(), *PanelCc = new TPanel(), *PanelCd = new TPanel();
    TPanel *PanelCe = new TPanel(), *PanelCf = new TPanel(), *PanelCg = new TPanel(), *PanelCh = new TPanel();
    TPanel *PanelDa = new TPanel(), *PanelDb = new TPanel(), *PanelDc = new TPanel(), *PanelDd = new TPanel();
    TPanel *PanelDe = new TPanel(), *PanelDf = new TPanel(), *PanelDg = new TPanel(), *PanelDh = new TPanel();
    // golden ClearCntButton[][] (btnClearAa..Dh) is written only by bucket
    // (b)'s btnClearAaClick and never read by any (a) method -- NOT declared
    // here (would be dead weight; matches the "leaner subset" DEVIATION).

    // -- Vibration motor run-time tab (golden :152-165) ----------------------
    TTabSheet *tsVibration        = new TTabSheet();
    TPanel    *palVibratorHP1      = new TPanel();
    TPanel    *palVibratorSht1     = new TPanel();
    TPanel    *palVibratorSht2     = new TPanel();
    TPanel    *palVibratorUnloader = new TPanel();

    // -- Function tab / one-click offset-limit (golden :166-167, :373-378) ---
    TTabSheet *tsFunction        = new TTabSheet();
    TGroupBox *gbSetOffsetLimit  = new TGroupBox();
    TEdit     *edtSetXYOffsetLimit = new TEdit();
    TEdit     *edtSetZOffsetLimit  = new TEdit();

    // -- Smart Diagnostic tab visibility flag (golden :168) ------------------
    TTabSheet *tsSmartDiagnostic = new TTabSheet();

    // -- Socket Count tab (golden :172-173) ----------------------------------
    TTabSheet   *tsSocketCount = new TTabSheet();
    TStringGrid *sgSocketCount = new TStringGrid(4, 33);

    // -- Contact-count-alarm shared edit (golden :329-330) -------------------
    TLabel *labContactCountAlarm    = new TLabel();
    TEdit  *editContactCountAlarm   = new TEdit();

    // -- Pre-Alarm Cylinder tab (golden :379-389) ----------------------------
    TTabSheet            *tsCylinderView       = new TTabSheet();
    TfStartConditionGrid *strngrdCylinderView  = new TfStartConditionGrid(4, 10);

    // -----------------------------------------------------------------------
    //  Wave A translated methods (bodies: cStartCondition.cpp, this wave)
    // -----------------------------------------------------------------------
    void FormShow();                                    // golden :106-536 (Sender dropped, see below)
    void DoIniDataToForm();                              // golden :1009-1027
    void pgLifeTimeChange();                             // golden :1029-1047 (Sender dropped)
    int  GetCylinderIndexByName(AnsiString sCylinderName); // golden :1479-1487
    void UpdateCylinderScreen(bool bRefrash = false);    // golden :1489-1589
    void LoadCylinderLife();                             // golden :1591-1653
    void strngrdCylinderViewDrawCell(int ACol, int ARow); // golden :1418-1450 (Sender/Rect/State dropped, GATE (SC5) -- entire body #if 0)
};

// DEVIATION: FormShow/pgLifeTimeChange drop their golden TObject* Sender
// parameter -- neither body reads it, matching every other translated form
// facade's identical simplification in this tree (forms/fSpeed.h's own
// FormShow/FormClose DEVIATION note cites the same rationale).

// AI(W906-FW3-StartCond-WA) 20260819: integration-pending -- NO
// `extern TfStartCondition *fStartCondition;` here. See INTEGRATION-PENDING
// note above.

#endif // FORMS_FSTARTCONDITION_H
