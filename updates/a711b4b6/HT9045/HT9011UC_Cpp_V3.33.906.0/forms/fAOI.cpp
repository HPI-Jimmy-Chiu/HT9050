// =============================================================================
//  forms/fAOI.cpp  --  definitions for the FrmAOI facade
//
//  AI(W906-W7-L2) 20260803: first home for TFrmAOI.  forms/fAOI.h carries the
//  golden citations, the TFrmAOI-vs-TfAOI naming deviation, and -- most
//  importantly -- the full argument for why this global is NOT initialised to
//  NULL despite the W7-L2 brief asking for NULL.  Read that block before
//  changing the line at the bottom of this file.
// =============================================================================
#include "forms/fAOI.h"
#include <cstdlib>   // atoi (TFrmAOI::CheckFailBin, golden fAOI.cpp:3719-3723)

// bSimulateTopBtm=false: golden's TFrmAOI ctor (fAOI.cpp:128) does not assign
// the member at all, so false is what VCL's instance zero-fill gives golden.
// Written explicitly here so the value is reviewable and so this facade does
// not depend on zero-init for a heap-allocated object.
TFrmAOI::TFrmAOI() : bSimulateTopBtm(false) {}

// -----------------------------------------------------------------------------
//  A LIVE INSTANCE, NOT NULL -- deliberate, and contrary to the W7-L2 brief.
//  Golden HT9045.cpp:233 `Application->CreateForm(__classid(TFrmAOI), &FrmAOI);`
//  is unconditional (verified by reading the whole CreateForm list,
//  HT9045.cpp:166-283: 118 calls, zero guards).  FrmAOI is therefore non-NULL on
//  every real machine by the time SoftStart can be set, so golden ckernel.cpp
//  :502's `if(FrmAOI!=NULL)` is TRUE and :504 runs.  A NULL here would have made
//  the ported :504 permanently unreachable while the translation claimed
//  fidelity.  forms/fAOI.h gives the full argument and the measurement showing
//  this choice carries no behavioural risk (bSimulateTopBtm has no reader
//  anywhere in golden).
//
//  NOTE FOR THE ckernel.cpp TRANSLATION: keep golden's `if(FrmAOI!=NULL)` guard
//  verbatim.  It is golden's line; it simply evaluates true here, as it does on
//  a real machine.  Do not "simplify" it away on the grounds that this pointer
//  is now known non-NULL.
// -----------------------------------------------------------------------------
TFrmAOI *FrmAOI = new TFrmAOI();

// =============================================================================
//  AI(W906-FW3-AOI1) 20260827: FW3-AOI1 -- first read-only translation pass
//  over golden fAOI.cpp (8,525 lines / 174 census functions; measured this
//  wave, tools/census/coverage_probe.py + span_sanity.py both 174/174, 0
//  missed extractions). 15 functions landed below. See forms/fAOI.h for the
//  new includes/widget stand-ins/global subset this required.
//
//  WHY ONLY 15 OF 174: this file is dominated by AOI camera motion (DoMoveXY_
//  ScannerAOI etc), the TopBottomInspect socket-command state machine
//  (TTopBottomInspect -- ~80 methods, none landed this wave: standing up the
//  class needs its own struct/enum skeleton and a TAOISocket type, judged
//  out of scope for a first pass that also has to leave room for the
//  landmine-checking below), file saves (SaveAOISummaryReport, AddAOIRecord),
//  and alarms (ShowErrorMessage). Full per-function exclusion reasoning is in
//  the wave report, not restated per-function here.
//
//  TWO PROVENANCE LANDMINES FOUND AND AVOIDED (see wave report for detail):
//   1. DoContinuousFailBySitePADView/BySiteBGAView (golden :884-897/:917-930)
//      read bVitroxPADViewUse/bVitroxBGAViewUse, which only DrawSitePanelVitrox1/
//      2 populate -- both excluded this wave (custom TTMyTray grid widget, not a
//      simple field). Landing the reader without its only writer would silently
//      always take the "reset to 0" branch.
//   2. The ENTIRE continuous-fail-counter family (DoContinuousFailByArmPADView/
//      ByArmBGAView, and TFrmAOI::DoContinuousFailBySiteScanAOI/ByArmScanAOI)
//      compares a counter against a tAOISetup.iContAlarmCount*/iScanAOIAlarmCount*/
//      iTopScanAOIAlarmCount* threshold. Grepped the WHOLE port tree (`grep -rn
//      'iContAlarmCountByArm\s*='` etc, 20260827) -- ZERO writers anywhere, for
//      any of the 6 threshold fields. A threshold of 0 makes `counter>=threshold`
//      TRUE on literally every call (counter is never negative), so every one of
//      these 4 functions would silently latch its *FailCont flag true on the
//      FIRST call regardless of Res -- not a benign "unconfigured default", a
//      wrong answer. All 4 excluded on this basis (on top of landmine #1 for the
//      two BySite variants). See KNOWLEDGE.md's "解gate前先查值從哪來" gotcha --
//      same shape, caught before landing this time.
//
//  ONE MORE FUNCTION DRAFTED THEN WITHDRAWN -- A LINK-BOUNDARY CATCH, NOT A
//  PROVENANCE ONE. GetAOIFailBin (golden :2892-2946, RogerYang 20251120) is
//  pure logic (reads ScannerAOIIF/Prod/TrayForm/OutArmSuck, all read-only,
//  returns an int bin, writes nothing) and was written, `-fsyntax-only`
//  clean, then pulled after `nm --undefined-only -C` on the compiled .o
//  showed `OutArmSuck` undefined and a check of CMakeLists.txt found its
//  only live definition (aHotPlateSubstrate.cpp:83 `TMyKitSuck OutArmSuck;`)
//  compiled into ht9045_sm (CMakeLists.txt:1432), which forms/fAOI.cpp's own
//  target, ht9045_forms, does NOT link (CMakeLists.txt:714: vclcompat +
//  ht9045_globals + ht9045_core only; the reverse edge is a documented
//  target cycle). ScannerAOIIF/TrayForm/Prod are all fine -- all three are
//  defined in cprod.cpp:10/14/103, which IS ht9045_globals. OutArmSuck alone
//  is the blocker. Reported so a future wave that ports enough of
//  TTopBottomInspect/ckernel to need OutArmSuck from ht9045_forms doesn't
//  have to re-discover this; the withdrawn draft is in this wave's report,
//  not left half-landed here.
// =============================================================================

// -----------------------------------------------------------------------------
//  Globals mirroring golden fAOI.cpp:44-126 -- ONLY the ones the 8 free
//  functions below touch.  Initial values copied verbatim from golden.
// -----------------------------------------------------------------------------
int iTopViewTask=1;                    // golden fAOI.cpp:45, extern fAOI.h:511
int iTopViewInspectionTask=1;          // golden fAOI.cpp:46, extern fAOI.h:512
int iPADViewTask=1;                    // golden fAOI.cpp:47, extern fAOI.h:513
int iPADViewInspectionTask=1;          // golden fAOI.cpp:48, extern fAOI.h:514
int iBGAViewTask=1;                    // golden fAOI.cpp:49, extern fAOI.h:515
int iBGAViewInspectionTask=1;          // golden fAOI.cpp:50, extern fAOI.h:516
int iScanAOITask=1;                    // golden fAOI.cpp:52, extern fAOI.h:518
int iScannerAOIInspectionTask=1;       // golden fAOI.cpp:53, extern fAOI.h:519

// golden fAOI.cpp:104/107-109 -- InitScannerAOIInspection's own reset targets.
// Not extern in golden fAOI.h either (file-private-by-convention); matched here.
int iScanLGAStep=0;                    // golden fAOI.cpp:104 //Ifor 20211022 add: AOI LGA Mode
bool bplus=true;                       // golden fAOI.cpp:107 //Richard 20220817 : 判斷加or減
int irowX=0;                           // golden fAOI.cpp:108
int icolumnY=0;                        // golden fAOI.cpp:109

// golden fAOI.cpp:116/115 -- InitTopScannerAOIInspection / InitialTopScanAOITask's
// own reset targets. Also not extern in golden fAOI.h (only the FUNCTION
// InitTopScannerAOIInspection is, fAOI.h:527 -- reproduced in forms/fAOI.h).
int iTopScannerAOIInspectionTask=1;    // golden fAOI.cpp:116
int iTopScanAOITask=1;                 // golden fAOI.cpp:115

// //AI(W906-FRW-S69) 20260926: golden fAOI.cpp:63-64／:73-74（Alick 20160713／20160714）Dut on/off 四張表。
// 讀寫檔在 FileRW/AOISetup.gen.inc（fAOI_ReadFile／spbSaveClick），AOI 執行期（Jimmy）將來也在本檔用 —— 同一份，
// 所以定義在 golden 的家；宣告見 forms/fAOI.h。golden 沒給初值（全域零初值 = false）。
bool bVitroxBGAViewMap[MAX_ARM_Row][MAX_ARM_Col];                               // golden fAOI.cpp:63 //Alick 20160713
bool bVitroxBGAViewUse[MAX_ARM_Row][MAX_ARM_Col];                               // golden fAOI.cpp:64 //Alick 20160713
bool bVitroxPADViewMap[MAX_ARM_Row][MAX_ARM_Col];                               // golden fAOI.cpp:73 //Alick 20160713
bool bVitroxPADViewUse[MAX_ARM_Row][MAX_ARM_Col];                               // golden fAOI.cpp:74 //Alick 20160713

//------------------------------------------------------------------------------
// golden fAOI.cpp:269-285. Pure: reads FrmAOI->ComboBox1->Text (now a
// TComboBox* stand-in, forms/fAOI.h), returns an int bin index. No golden
// header declaration (file-private in golden too). Empty-string default on
// ComboBox1->Text matches none of the five branches, so iResult's local
// initialiser (0) is what callers see pre-configuration -- same "offline
// default, no golden branch silently mis-selected" shape as bSimulateTopBtm.
//------------------------------------------------------------------------------
int DoViewBinFunction()
{
    int iResult=0;

    if(FrmAOI->ComboBox1->Text=="Auto1")
        iResult=0;
    else if(FrmAOI->ComboBox1->Text=="Auto2")
        iResult=1;
    else if(FrmAOI->ComboBox1->Text=="Auto3")
        iResult=2;
    else if(FrmAOI->ComboBox1->Text=="Fix1")
        iResult=3;
    else if(FrmAOI->ComboBox1->Text=="Fix3")
        iResult=5;

    return iResult;
}
//------------------------------------------------------------------------------
// golden fAOI.cpp:516-519. Trivial task-counter reset.
//------------------------------------------------------------------------------
void InitTopViewInspection()
{
    iTopViewInspectionTask=1;
}
//------------------------------------------------------------------------------
// golden fAOI.cpp:655-658. Trivial task-counter reset.
//------------------------------------------------------------------------------
void InitDoTopViewFunction()
{
    iTopViewTask=1;
}
//------------------------------------------------------------------------------
// golden fAOI.cpp:879-882. Trivial task-counter reset.
//------------------------------------------------------------------------------
void InitPADViewInspection()
{
    iPADViewInspectionTask=1;
}
//------------------------------------------------------------------------------
// golden fAOI.cpp:1128-1131. Trivial task-counter reset.
//------------------------------------------------------------------------------
void InitDoPADViewFunction()
{
    iPADViewTask=1;
}
//------------------------------------------------------------------------------
// golden fAOI.cpp:1358-1361. Trivial task-counter reset.
//------------------------------------------------------------------------------
void InitBGAViewInspection()
{
    iBGAViewInspectionTask=1;
}
//------------------------------------------------------------------------------
// golden fAOI.cpp:1555-1558. Trivial task-counter reset.
//------------------------------------------------------------------------------
void InitBGAViewFunction()
{
    iBGAViewTask=1;
}
//------------------------------------------------------------------------------
// golden fAOI.cpp:1834-1841. Trivial reset of 4 task/LGA-mode counters.
//------------------------------------------------------------------------------
void InitScannerAOIInspection()
{
    iScannerAOIInspectionTask=1;
    iScanLGAStep=0;                                                             //Ifor 20211022 add: AOI LGA Mode
    bplus   =true;                                                              //判斷加or減
    irowX   =0;                                                                 //Richard 20220817:Add Scanner_AOI add check Big Size IC Function
    icolumnY=0;
}
//------------------------------------------------------------------------------
// golden fAOI.cpp:1990-1993. Trivial task-counter reset. Golden also
// declares this one extern (fAOI.h:527) -- reproduced in forms/fAOI.h.
//------------------------------------------------------------------------------
void InitTopScannerAOIInspection()
{
    iTopScannerAOIInspectionTask=1;
}
//------------------------------------------------------------------------------
// golden fAOI.cpp:2122-2125. Trivial task-counter reset.
//------------------------------------------------------------------------------
void InitialScanAOITask()
{
    iScanAOITask=1;
}
//------------------------------------------------------------------------------
// golden fAOI.cpp:2384-2387. Trivial task-counter reset.
//------------------------------------------------------------------------------
void InitialTopScanAOITask()
{
    iTopScanAOITask=1;
}
//------------------------------------------------------------------------------
// golden fAOI.cpp:2892-2946 GetAOIFailBin -- DRAFTED, THEN WITHDRAWN this
// wave for a link-boundary block, not a correctness one. See the header
// banner above (search "LINK-BOUNDARY CATCH") and forms/fAOI.h for the full
// note: it needs OutArmSuck, whose only live definition is compiled into
// ht9045_sm, which ht9045_forms (this file's own target) does not link.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// golden fAOI.cpp:3715-3736 (TFrmAOI::CheckFailBin). Pure: 4 TEdit->Text +
// 3 TCheckBox->Checked reads, Prod.bIsPassBin[] (global, READ only), returns
// bool, writes nothing. GOLDEN QUIRK preserved verbatim: iBin[0] is declared
// but never assigned or read (loop below runs i=1..4 only, via
// `i<sizeof(iBin)/sizeof(int)` which is 5 -- golden's own off-by-one-looking
// index scheme, not this port's).
//------------------------------------------------------------------------------
bool TFrmAOI::CheckFailBin()
{
    int iBin[5];

    iBin[1]=atoi(edt_FailPADView->Text.c_str());
    iBin[2]=atoi(edt_FailBGAView->Text.c_str());

    iBin[3]=atoi(edt_PassFailBGAPADView->Text.c_str());
    iBin[4]=atoi(edt_FailFailBGAPADView->Text.c_str());

    if(cbEnabledBGAView->Checked || cbEnabledPADView->Checked || cbEnabledTopView->Checked)
    {
        for(int i=1; i<sizeof(iBin)/sizeof(int); i++)
        {
            if(Prod.bIsPassBin[iBin[i]])
            {
                return false;
            }
        }
    }
    return true;
}
//------------------------------------------------------------------------------
// golden fAOI.cpp:4429-4443 (TFrmAOI::UpdateAOIFailBinTypetoForm, private in
// golden too, fAOI.h:408). Pure widget-to-widget update: reads
// rgAOIFailBinType->ItemIndex, writes lblAOIBinSel1->Caption/
// lblAOIBinSel2->Visible/cbAOIFialAndTestPass->Visible. No config threshold,
// no landmine -- ItemIndex's zero-init default (0) selects golden's first
// branch, a real golden state (not a degenerate one). Currently unwired: its
// only golden caller, rgAOIFailBinTypeClick, is a *Click handler and is
// excluded this wave by the hard "no event handlers" rule.
//------------------------------------------------------------------------------
void TFrmAOI::UpdateAOIFailBinTypetoForm()                                      //RogerYang 20251120 : 新增AOI測試後分bin也考慮測試結果的選項
{
    if(rgAOIFailBinType->ItemIndex==0)
    {
        lblAOIBinSel1->Caption          ="AOI Fail :";
        lblAOIBinSel2->Visible          =false;
        cbAOIFialAndTestPass->Visible   =false;
    }
    else
    {
        lblAOIBinSel1->Caption          ="AOI Fail + Test Fail :";
        lblAOIBinSel2->Visible          =true;
        cbAOIFialAndTestPass->Visible   =true;
    }
}
//------------------------------------------------------------------------------
// golden fAOI.cpp:4472-4483 (TFrmAOI::SetSimulationState). Sets bSimulate +
// btnSimulateTopBtm->Caption. See forms/fAOI.h for why bSimulate defaults to
// false here (golden's own ctor only sets it inside a gate this facade does
// not reproduce).
//------------------------------------------------------------------------------
void TFrmAOI::SetSimulationState(bool bswitch)
{
    bSimulate=bswitch;
    if(bswitch)
    {
        btnSimulateTopBtm->Caption="Running...";
    }
    else
    {
        btnSimulateTopBtm->Caption="Run";
    }
}
//------------------------------------------------------------------------------
// golden fAOI.cpp:4516-4519 (TFrmAOI::ShowTopBtnTask). Single Caption write.
//------------------------------------------------------------------------------
void TFrmAOI::ShowTopBtnTask(AnsiString sMsg)
{
    blTopBtmTask->Caption="Task:"+sMsg;
}
