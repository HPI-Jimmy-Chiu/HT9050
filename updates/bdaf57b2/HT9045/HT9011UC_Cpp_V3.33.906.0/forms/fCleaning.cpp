// =============================================================================
//  forms/fCleaning.cpp  --  definitions for the fCleaning facade
//
//  AI(W906-W7-F0) 20260728: split out of FormsFacade.cpp by the W7-F0 refactor
//  (docs/W7_UI_ARCHITECTURE_PLAN.md SS6-F0-d).  Body moved VERBATIM.
// =============================================================================
#include "forms/fCleaning.h"
// AI(W906-FW3-CLN1) 20260827: FW3-CLN1 wave includes -- every global the 30
// methods delivered this wave dereference lives in one of these (see forms/
// fCleaning.h's own banner for the per-name provenance). Same "list what each
// include is for" convention as forms/fLotInfo.cpp:18-36.
#include "cmydef.h"             // CUSTOMER_CODE/CC_Greatek/N_INTEGER/N_DOUBLE (cmydef.cpp, ht9045_globals)
#include "cprod.h"              // TestIF_File/InputLimit/TrayForm/UserDefForm_File[] (cprod.cpp, ht9045_globals)
#include "Config.h"             // IniConfig (Config.cpp, ht9045_globals)
#include "common.h"             // OnlyNumberInPut/OnlyNumberAndDotInPut (common.cpp, ht9045_core)
#include "forms/fQwertyKey.h"   // fQwertyKey->ShowQwertyKey (real since FW-QWKEY1, forms/fQwertyKey.cpp, ht9045_forms)
#include "CosFunction.h"        // CosFunction.bSmartAutoClean (CosFunction.cpp, ht9045_globals)
#include "forms/fLotInfo.h"     // fLotInfo->RefreshYieldMonitor (real, forms/fLotInfo.cpp, ht9045_forms)

// --- W906-AutoCleanFoundation ADD: TfCleaning -------------------------------
TfCleaning::TfCleaning() : iDeviceCount(0), bResetCleanCount(false), b1x2SiteAbClosePutDummy(false)
{
    edCleaningCount = new TfLotInfoEdit();
    // AI(W906-FW3-CLN1) 20260827: new widget/data members below. Ctor safety
    // rule (docs/KNOWLEDGE.md "static-init ctor 不可碰 NULL 全域"): fields
    // only, `new` of THIS object's own widget stand-ins only -- none of these
    // deref any other global, so this stays outside the SIOF class of risk
    // that cost this tree 88/134 ctest SEGFAULTs previously.
    edPinSingleGf = new TfLotInfoEdit();
    edPinSingleN  = new TfLotInfoEdit();
    XCT1          = new TfLotInfoEdit();
    XCT2          = new TfLotInfoEdit();
    OutArmSpeed   = new TfLotInfoEdit();
    lblInArm      = new TLabel();
    lblInArmZ     = new TLabel();
    pgCleanType   = new TPageControl();
    udDeviceCT    = new TfCleaningUpDown();
    iPosTemp      = 0;      // golden ZeroInitVclFields -- ctor never sets it (uCleaning.h:338)
    b12SiteRun2x4 = false;  // golden ZeroInitVclFields -- ctor never sets it (uCleaning.h:360)
    // AI(W906-FW3-CLN2) 20260827: same ctor safety rule -- fields and this
    // object's own widget stand-ins only, no other global dereferenced.
    edAlarmCount  = new TfLotInfoEdit();
    sbCleanExit   = new TSpeedButton();
}
TfCleaning *fCleaning = new TfCleaning();

// =============================================================================
//  FW3-CLN1 wave (20260827) -- 31 of 72 golden TfCleaning:: methods delivered.
//  (Was written as "30" while the wave was in flight; ResetSmartAutoClean was
//  cleared on a second pass and appended afterwards. Main loop counted the
//  definitions itself with comments stripped: 32 column-0 TfCleaning::
//  definitions = 1 ctor (line 21, pre-existing) + 31 methods.)
//
//  SELECTION CRITERION, applied per the task brief's hard boundary: link-safe
//  from ht9045_forms (which links ONLY vclcompat + ht9045_globals + ht9045_core
//  -- CMakeLists.txt:729-736 -- and reaches NEITHER ht9045_sm NOR ht9045_motor/
//  ht9045_io/ht9045_db), no file write, no machine action, no outbound comms.
//  Measured this wave (20260827) which golden .cpp each dependency's BODY
//  lives in and which CMake target that .cpp belongs to, via CMakeLists.txt
//  add_library block boundaries -- not by name/intuition. The single biggest
//  practical consequence: fSecurity/fContact/fConfiguration/fSetup/DoInArm_
//  9045_Type/SetWorkParameter/ShowMyMessage(non-static)/Barcode_Reader all
//  have their real bodies in cSecurity.cpp/cContact.cpp(forms/fContact.cpp)/
//  cConfiguration.cpp/cinitial.cpp -- every one inside the ht9045_sm
//  add_library block (CMakeLists.txt:1368-2419) -- so ANY translated body that
//  calls them would compile but fail to LINK, and since the fCleaning global
//  is already referenced (3 live call sites), this whole .o is ALWAYS pulled
//  into every link -- an unresolved call from an ACTIVE method here would
//  break EVERY target that links ht9045_forms, not just a hypothetical future
//  caller. fQwertyKey is the one exception: forms/fQwertyKey.cpp is itself
//  inside the ht9045_forms add_library block (CMakeLists.txt:699), so
//  fQwertyKey->ShowQwertyKey(...) is link-safe.
//
//  TRANSITIVE RULE (forms/fContact.h precedent, applied the same way here): a
//  method that calls an excluded method is itself excluded. This is why
//  DrawAutoClean/SetDeviceInTray/SetCleanCellValue/SetDeviceMaxMin/
//  GetMinCleanPadCount/CleanPadCountCanSupport2Arm/rgCleanKitTypeClick/
//  cbbCleanPadCountChange/XPitch2Change/btIncludeClick/udDeviceCTClick are ALL
//  excluded even though several of them look UI-only in isolation -- see the
//  agent's reply banner for the exact call chain each one is excluded through.
//
//  Full 72/72 exclusion ledger, GATE register, and new-symbol provenance are
//  in this wave's reply to the coordinating agent (not duplicated into this
//  file to avoid the two-copies-drift risk forms/fSetup.h's own banner already
//  states for the identical situation).
// =============================================================================

// ---------------------------------------------------------------------------
//  ShowTranGfToN -- golden :1676-1683.
//  Pure arithmetic + AnsiString formatting over this object's own two TEdit
//  widgets. No external singleton, no global write.
// ---------------------------------------------------------------------------
void TfCleaning::ShowTranGfToN()
{
    double f=atof(edPinSingleGf->Text.c_str());                                // golden :1678
    f=f*9.8/1000.0;                                                            // golden :1679
    AnsiString str;                                                           // golden :1680
    str.sprintf("%8.4f", f);                                                   // golden :1681
    edPinSingleN->Text=str.Trim();                                             // golden :1682
}

// ---------------------------------------------------------------------------
//  SetArmCaption -- golden :55-63 (golden declares it `private:`; collapsed to
//  public per this tree's established D-6 convention, e.g. forms/fContact.h).
//
//  GATE (one line, inside an otherwise-ACTIVE method): golden :61
//  `OutArmSpeed->Hint="AutoClean_InArm Speed";` -- vclcompat::TEdit (and
//  TControl generally) has no ->Hint property anywhere in the port (`grep -rn
//  "Hint" vclcompat/Controls.h` -- 0 hits, 20260827; the same gap this wave's
//  task brief calls out for TSpeedButton::Hint extends to TEdit). Purely
//  cosmetic tooltip text -- dropping it changes no persisted value and no
//  control flow. Same "drop the one unsupported cosmetic write, keep the rest
//  ACTIVE" shape as forms/fSecurity.h's GATE (SEC5)/(SEC6).
// ---------------------------------------------------------------------------
void TfCleaning::SetArmCaption()
{
    lblInArm->Caption="Input Arm ";                                            // golden :57
    lblInArmZ->Caption="Input Arm Z";                                          // golden :58
    if(CUSTOMER_CODE==CC_Greatek)                                              // golden :59
    {
        // GATE: golden :61 OutArmSpeed->Hint=... -- see banner above.
    }
}

// ---------------------------------------------------------------------------
//  rgKitPositionClick -- golden :1886-1889. Whole body is a call into the
//  now-delivered SetArmCaption().
// ---------------------------------------------------------------------------
void TfCleaning::rgKitPositionClick(TObject *Sender)
{
    (void)Sender;
    SetArmCaption();                                                           // golden :1888
}

// ---------------------------------------------------------------------------
//  btnResetCleanCountClick -- golden AutoClean/uCleaning.cpp:2095-2117.
//  AI(W906-ST-S3-B2b) 20260918: DOCUMENTED NO-OP STUB, not a translation.
//
//  WHY IT APPEARED NOW. Its only call site is csystem.cpp:17614, inside
//  `#ifdef SOFT_SIMULTE` ("clean pad was changed, so reset the counter"). This
//  tree's convention was that SOFT_SIMULTE is never #defined, so the
//  preprocessor deleted the call before the linker could miss the method. The
//  user turned SOFT_SIMULTE on 20260918.
//
//  WHY A STUB AND NOT THE REAL BODY. Golden's 23 lines are a write path with an
//  outbound message, not UI. Counted from golden:
//    :2097  Barcode_Reader(bcAutoClean)     -- early return when it answers 0
//    :2102  ReadWriteAutoCleanCount(false,true)  -- WRITES the clean count
//    :2103  fShowBinSelect->ed_AutoCleanCount->Text = 0
//    :2104  fCleaning->edCleaningCount->Text = 0
//    :2105  btnResetCleanCount->Enabled = false
//    :2107  SearchCleanNum()
//    :2111  SetAutoCleanICCount(false)
//    :2113  TestIF_File.iIndexArmAutoCleanCnt = 0
//    :2114  fCleaning->edIndexArmAutoCleanCnt->Text = 0
//    :2115  EventReport(SECS_EVENT.AutoCleanClearCount)  -- OUTBOUND SECS event
//  Translating it faithfully means deciding what a counter write and an
//  outbound SECS event should do in a simulation build. That is a decision,
//  not a transcription, and it is not what the user asked for today (his
//  words: "我現在只想讓 start 和 pause 運作").
//
//  BEHAVIOUR DELTA ON A SIMULATED RUN: after a simulated clean-pad change the
//  AutoClean counter is NOT reset, so it keeps counting up from its previous
//  value and will reach its alarm threshold earlier than golden would. Nothing
//  is written and no SECS event is sent. Visible, bounded, and reversible by
//  translating the body.
// ---------------------------------------------------------------------------
void (*W906_ResetCleanCountHook)() = 0;  void TfCleaning::btnResetCleanCountClick(TfCleaning *Sender)   //AI(W906-W170) 20261008 (NB2-1): W-170 -- the hook is set by FileRW_Cleaning_Boot (FileRW/TestIF_File_Cleaning.cpp, wb_serve only) to the golden body; 0 (unit tests that link this facade alone) = the old no-op
{
    (void)Sender;  if (W906_ResetCleanCountHook) W906_ResetCleanCountHook();   //AI(W906-W170) 20261008 (NB2-1): W-170 (R265 = A, RULINGS_20261001 #0) -- the STUB note above is history: golden uCleaning.cpp:2095-2117 (V912 :2116-2138) runs in the C route (FileRW/TestIF_File_Cleaning.gen.inc CL_btnResetCleanCountClick) -- Barcode_Reader(bcAutoClean), ReadWriteAutoCleanCount(false,true), SearchCleanNum, SetAutoCleanICCount, iIndexArmAutoCleanCnt=0, EventReport(AutoCleanClearCount)
}

// ---------------------------------------------------------------------------
//  XCT1Change -- golden :2119-2191. Pure UI-state arithmetic over this
//  object's own XCT1/XCT2/pgCleanType widgets, TestIF_File, this object's own
//  b12SiteRun2x4 member, and the ht9045_globals UserDefForm_File[]/TrayForm
//  pair (cprod.cpp). Golden's own commented-out dead branches (:2126-2139) and
//  trailing commented-out SetDeviceMaxMin() call (:2190) are kept VERBATIM as
//  comments -- SetDeviceMaxMin is itself excluded this wave (it transitively
//  reaches GetMinCleanPadCount/DrawAutoClean, both ht9045_sm/MOT[]-dependent),
//  so translating that comment into a real call is out of scope regardless of
//  golden having disabled it for an unrelated reason.
// ---------------------------------------------------------------------------
void TfCleaning::XCT1Change(TObject *Sender)
{
    (void)Sender;
    int iData1=atoi(XCT1->Text.c_str());                                       // golden :2121
    int iData2=atoi(XCT2->Text.c_str());                                       // golden :2122

    if(pgCleanType->ActivePageIndex==1)                                        // golden :2124  //Steven 20220128 : Fixed for Auto Clean X Item亂跳
    {
//        if(TestIF_File.iTestMode==TriSite1X3 ||                              //Steven 20240416 : 修改1x3使用3ea Clean Pad
//           TestIF_File.iTestMode==_6Site2X3N)
//        {
//            iCount=3;
//        }
//        else if(TestIF_File.iTestMode==_6Site2X3)                            //Steven 20240416 : 修改2x3使用6ea Clean Pad
//        {
//            iCount=6;
//        }
//        else if(TestIF_File.iTestMode==_10Site2X5 &&                         //Steven 20240416 : 修改2x5使用10ea Clean Pad
//                b12SiteRun2x4==false)
//        {
//            iCount=10;
//        }

        if(TestIF_File.iTestMode==_12Site2X6 &&                                // golden :2141  //Steven 20250809 : 修改2x6使用12ea Clean Pad
           b12SiteRun2x4==false &&
           (iData2==6 || iData2==8 || iData2==4 || iData2==12))
        {
            ;
        }
        else if(iData2<8 && iData2>=4)                                         // golden :2147  //Alick 20170329 (wei) modify 原本用字串比對數字有錯
        {
            XCT2->Text=4;
        }
        else if(iData2%4==0)                                                   // golden :2151  //kevin 20220906
        {
            XCT2->Text=(iData2/4)*4;
        }
        else
        {
            XCT2->Text=UserDefForm_File[TrayForm.Loader.iTrayType].XDivision;   // golden :2157
        }
    }
    else
    {
        if(iData1<8 &&
           iData1>=4)                                                          // golden :2162-2163  //Alick 20170329 (wei) modify 原本用字串比對數字有錯
        {
            XCT1->Text=4;
        }
        else if(TestIF_File.iTestMode==_10Site2X5 &&                           // golden :2167  //Steven 20220125 : for 2x5 mode
                iData1>=10)
        {
            XCT1->Text=10;
        }
        else if(TestIF_File.iTestMode==_12Site2X6 &&                           // golden :2172  //Steven 20250809 : 修改2x6使用12ea Clean Pad
                b12SiteRun2x4==false &&
                (iData1==6 || iData1==8 || iData1==4 || iData1==12))
        {
            ;
        }
        else if((TestIF_File.iTestMode==_16Site2X8 ||                          // golden :2178
                 TestIF_File.iTestMode==_32Site4X8N) &&
                iData1==12)                                                    // golden :2180  //Steven 20250912 : for 16site auto clean with 12x16 Hot plate
        {
            XCT1->Text=12;
        }
        else if(iData1>=8)                                                     // golden :2184
        {
            XCT1->Text=8;
        }
    }

//    SetDeviceMaxMin();                                                       //Jimmychiu 20250121 : 調換位置，避免換工單檢測錯誤   -- golden :2190, EXCLUDED (see banner)
}

// ---------------------------------------------------------------------------
//  udDeviceCTChangingEx -- golden :2242-2246.
//  DEVIATION: golden signature `(TObject *Sender, bool &AllowChange, short
//  NewValue, TUpDownDirection Direction)` -- ALL FOUR parameters dropped. None
//  is read in golden's own body (verified by direct read this wave);
//  `TUpDownDirection` additionally has zero port anywhere in this tree
//  (`grep -rn "TUpDownDirection" --include=*.{h,cpp} .` -- 1 hit tree-wide, a
//  comment in cConfiguration.cpp:6465 recording the identical drop for its own
//  udD46Click sibling, 20260827), so even keeping the other three would still
//  block on this one. Same "drop unread params that need an unported type"
//  shape as cConfiguration.cpp:6462's udD46Click.
// ---------------------------------------------------------------------------
void TfCleaning::udDeviceCTChangingEx()
{
    iPosTemp=udDeviceCT->Position;                                             // golden :2245  //2013-06-11 Dell Fix Clean pad 只能是Site number 倍數
}

// ---------------------------------------------------------------------------
//  XST1KeyPress -- golden :1874-1878. XCT1KeyPress -- golden :1880-1884.
//  edtLowYieldLimitKeyPress -- golden :2264-2269. All three are the same
//  digit-filter shape over OnlyNumberAndDotInPut/OnlyNumberInPut (common.cpp,
//  ht9045_core -- link-safe).
// ---------------------------------------------------------------------------
void TfCleaning::XST1KeyPress(TObject *Sender, char &Key)
{
    (void)Sender;
    if(OnlyNumberAndDotInPut(Key)==false)                                      // golden :1876
        Key=NULL;
}

void TfCleaning::XCT1KeyPress(TObject *Sender, char &Key)
{
    (void)Sender;
    if(OnlyNumberInPut(Key)==false)                                            // golden :1882
        Key=NULL;
}

void TfCleaning::edtLowYieldLimitKeyPress(TObject *Sender, char &Key)
{
    (void)Sender;
    if(OnlyNumberInPut(Key)==false)                                            // golden :2267
        Key=NULL;
}

// ---------------------------------------------------------------------------
//  ResetSmartAutoClean -- golden :2736-2745. Found link-safe on a SECOND pass
//  (missed in this wave's first dependency scan, which only grepped for
//  `fMain->`/`fSecurity`/etc. by name and did not initially check where
//  iACUseParam/iACSmartCount/iACSmartCount_CTF are DEFINED): all three are
//  cmydef.cpp globals (cmydef.h:5555-5557, ht9045_globals), CosFunction.
//  bSmartAutoClean is CosFunction.cpp (ht9045_globals), and fLotInfo->
//  RefreshYieldMonitor() is a REAL, DEFINED member of forms/fLotInfo.cpp --
//  itself inside the ht9045_forms add_library block (CMakeLists.txt:656) --
//  so calling it is link-safe (the already-passing test_AutoClean.exe /
//  full-tree gate build this wave confirms it, not just static reasoning).
// ---------------------------------------------------------------------------
void TfCleaning::ResetSmartAutoClean()
{
    if(CosFunction.bSmartAutoClean)                                            // golden :2738
    {
        iACUseParam=0;                                                         // golden :2740
        iACSmartCount=0;                                                       // golden :2741
        iACSmartCount_CTF=0;                                                   // golden :2742
        fLotInfo->RefreshYieldMonitor();                                       // golden :2743
    }
}

// =============================================================================
//  fQwertyKey->ShowQwertyKey(...) passthrough handlers (22).
//  Every one of these is, in golden, a single call into the on-screen keypad
//  with a fixed (or one-branch) argument list; forms/fQwertyKey.cpp is itself
//  inside the ht9045_forms link diet (CMakeLists.txt:699), so this whole group
//  is link-safe. Literal numeric arguments are transcribed EXACTLY as golden
//  wrote them, including two documented golden oddities (edTimeCTClick,
//  edtACSmart_ContactTimeMouseDown) -- see their own inline notes below.
// =============================================================================

void TfCleaning::edtBufferKitLTXClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iOffsetXYHigh, (double)InputLimit.iOffsetXYLow);   // golden :2576
}

void TfCleaning::edtHotplatePickOffsetClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iOffsetZHigh, (double)InputLimit.iOffsetZLow);   // golden :2581
}

void TfCleaning::edACContactCleanHeightClick(TObject *Sender)
{
    if(IniConfig.bSPILFunction)                                                // golden :2586  //JerryYang 20240111 : 矽品二林要求設定auto clean contact height上下限
    {
        fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_DOUBLE, 2, true, InputLimit.dAutoCleanContactHigh, InputLimit.dAutoCleanContactLow);   // golden :2588
    }
    else
    {
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iOffsetZHigh, (double)InputLimit.iOffsetZLow);   // golden :2592
    }
}

void TfCleaning::edtLowYieldLimitClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 100, 1);      // golden :2598
}

void TfCleaning::edtLowYieldCountClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 100000, 1);   // golden :2603
}

void TfCleaning::XST1Click(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 0.00, 1000.00);   // golden :2608  //Steven 20250812 : min 0.00
}

void TfCleaning::XCT1Click(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 100, 1);     // golden :2613
}

void TfCleaning::edACContactShiftHeightClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 0.01, 30.0);  // golden :2618
}

void TfCleaning::edDropOffset1Click(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_DOUBLE, 1, true, 10.0, 0.0);    // golden :2623
}

void TfCleaning::edDevicePicesClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, udDeviceCT->Max, udDeviceCT->Min);   // golden :2628
}

void TfCleaning::edAutoCleanAirForce_KgClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_DOUBLE, 2, true, 500.0, 0.0);   // golden :2633
}

void TfCleaning::edAutoCleanAirForce_NClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_DOUBLE, 2, true, 5000.0, 0.0);  // golden :2638
}

void TfCleaning::edtACSmartClick(TObject *Sender)                              //Sam 20230111 : Smart Auto Clean
{
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 10, 1);       // golden :2643
}

// GOLDEN NOTE: golden :2861 passes `(iDP=)5000, (bCheckRange=)5` into a
// signature typed `(int iDP=0, bool bCheckRange=false, ...)`. iDP=5000 is not
// a plausible decimal-place count and bCheckRange=5 truncates to `true` --
// almost certainly a copy-paste slip (compare edACSmartCTFMouseDown/
// edtACSmartMouseDown below, both `(1, 10, 1)`), but it is golden's own
// literal text and is translated verbatim per this tree's "照翻，並在//AI
// 註解寫下它為什麼看起來錯" rule -- not corrected.
void TfCleaning::edTimeCTClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, 1, 5000, 5);                      // golden :2861
}

void TfCleaning::edtIndexVacuumClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 10.0, 0.01);  // golden :2866
}

void TfCleaning::edACInitalContactCountClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 100, 0);      // golden :2876
}

void TfCleaning::edAdaptiveIntervalMaxMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Button; (void)Shift; (void)X; (void)Y;
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 10, true, 9999, 10);  // golden :2820
}

void TfCleaning::edAdaptiveIntervalMinMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Button; (void)Shift; (void)X; (void)Y;
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 10, true, 9999, 1);   // golden :2826
}

void TfCleaning::edAdaptiveIntervalAdjMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Button; (void)Shift; (void)X; (void)Y;
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 10, true, 1000, 1);   // golden :2832
}

void TfCleaning::edACSmartCTFMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Button; (void)Shift; (void)X; (void)Y;
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, 1, 10, 1);                        // golden :2838
}

// GOLDEN NOTE: golden :2845 passes double literals (0.01, 100.0, 0.0) into
// `(int iFunction, int iDP=0, bool bCheckRange=false, ...)` -- iFunction=0.01
// truncates to 0, which is neither N_INTEGER(1) nor N_DOUBLE(2) (cmydef.cpp:
// 350-351); iDP=100 and bCheckRange=(0.0!=0)=false follow the same truncation.
// Almost certainly meant `N_DOUBLE, 2, true, ...` (compare edACContactCleanHeightClick
// above, same "double field, 2 decimal places" shape) but this is golden's own
// literal text -- translated verbatim, not corrected, per this tree's standing
// "照翻並記錄" rule. `-Wconversion` is not part of ht9045_forms' warning set
// (CMakeLists.txt:742-743: -Wall -Wextra only), so this narrows silently, same
// as it silently narrows in golden's own BCB6 build.
void TfCleaning::edtACSmart_ContactTimeMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Button; (void)Shift; (void)X; (void)Y;
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, 0.01, 100.0, 0.0);                // golden :2845
}

void TfCleaning::edtACSmartMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Button; (void)Shift; (void)X; (void)Y;
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, 1, 10, 1);                        // golden :2851
}

// =============================================================================
//  FW3-CLN2 wave (20260827) -- the three functions FW3-CLN1 had already read,
//  dependency-checked and judged safe, and then simply did not write out.
//
//  They did not need a fresh safety review: CLN1's report labelled them
//  oversights explicitly, which is the only reason they are cheap now. Recorded
//  in docs/DEVLOG.md 20260827 XIV. Every dependency below was re-measured by
//  the main loop before translating -- "the previous wave said it was safe" is
//  not evidence, and this tree has a standing rule that absence-claims expire.
//
//  Re-measured 20260827 (main loop, not taken from the CLN1 report):
//    IniConfig.bE43AutoCleanUseHotplate  Config.h:608 (IniConfig -> cprod.cpp:50,
//                                        ht9045_globals) -- reachable
//    vclcompat::TSpeedButton::Down       vclcompat/Controls.h:484-491 -- exists
//    Close()                             no port to inherit; added as an offline
//                                        no-op on the facade, precedent
//                                        forms/fQwertyKey.h:366
//    golden field types                  uCleaning.h:35 `TEdit *edAlarmCount`,
//                                        uCleaning.h:166 `TSpeedButton *sbCleanExit`
// =============================================================================

void TfCleaning::YCT1Change(TObject *Sender)
{
    TEdit *ptr=(TEdit *)Sender;                                                // golden :1893

    if(IniConfig.bE43AutoCleanUseHotplate==false)                              // golden :1895
    {
        if(atoi(ptr->Text.c_str())>10)                                         // golden :1897 (kevin 20210911 change 4->10)
            ptr->Text=10;                                                      // golden :1898
    }
//  SetDeviceMaxMin();                                                         // golden :1900 -- commented out in golden itself
//                                                                                (Jimmychiu 20250121), kept as a comment so the
//                                                                                port does not silently gain a call golden dropped
}

void TfCleaning::sbCleanExitClick(TObject *Sender)
{
    (void)Sender;
    Close();                                                                   // golden :1870 -- offline no-op, see forms/fCleaning.h
    sbCleanExit->Down=false;                                                   // golden :1871
}

void TfCleaning::edCleanCountClick(TObject *Sender)
{
    // golden :2082. The range max is (edAlarmCount's value - 1), read at call
    // time -- NOT a constant. edAlarmCount has no writer in this build yet, so
    // the value is "" -> atoi 0 -> max -1. That is a degraded RANGE on a screen
    // keyboard this build never shows, and it is not consumed by any other
    // module, so it is disclosed rather than gated -- same criterion as
    // TfOffSetEdit::Name (20260827 XI) and unlike AOI1's latched fail flag.
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, (atoi(edAlarmCount->Text.c_str()))-1, 0);
}
