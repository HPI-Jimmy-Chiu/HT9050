// =============================================================================
//  cShowBinSelect.cpp  --  FW-3 queue item 2: TfShowBinSelect, WAVE A subset
//
//  Translation wave: FW-3 ShowBinSelect Wave A
//  Translator: AI(W906-FW3-ShowBinSelect-WA) 20260818
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/cShowBinSelect.cpp
//  (3,016 lines) + cShowBinSelect.h (524 lines), cp950/Big5. Decoded this
//  wave with `python3 -c "open(path,'rb').read().decode('cp950')"` -- 0
//  U+FFFD over both files (measured before any line below was written).
//
//  See forms/fShowBinSelect.h for the full WAVE A / WAVE B split, GATE
//  REGISTER, DEVIATION list (bootstrap ctor) and facade shape -- not
//  duplicated here to avoid the two files drifting apart.
//
//  ABSENCE-CLAIM TIMESTAMPS (commands + when run, this wave, before writing
//  the citing code below -- re-run at hand-off per project policy)
//  --------------------------------------------------------------------------
//    fCounterClear facade : `grep -rn "fCounterClear" --include=*.h .` --
//                            1 hit, Automation/SCK_ART_Remainder.h:71, itself
//                            a comment recording the same gap (20260818).
//    fSecurity facade     : `grep -rn "class TfSecurity" --include=*.h .` --
//                            0 hits anywhere (20260818).
//    fBinSel facade       : `grep -rn "fBinSel" --include=*.h .` -- only
//                            comment-only hits in forms/fMain.h (20260818).
//    fCleaning->btnResetCleanCountClick : `grep -n
//                            "btnResetCleanCountClick" forms/fCleaning.h` --
//                            0 hits (20260818).
//    fFixAICCD->UnloadAICntNG : `grep -n "UnloadAICntNG" forms/fFixAICCD.h`
//                            -- 0 hits (20260818).
//    fMain->StatusBar1    : `grep -n "StatusBar1" forms/fMain.h` -- 0 hits
//                            (20260818).
//    Application->MessageBoxA / MB_OKCANCEL / IDOK : no TApplication surface
//                            anywhere in vclcompat (same absence as
//                            forms/fContactCT.h's GATE (C2); 20260818).
//    ShowMyMessageBox_YES_NO : Automation/AGV_E84.h:58's own comment already
//                            records it "untranslated" (20260818).
//    HSys.BinDisCtrl (opaque): `grep -n BinDisCtrl database.h` -- declared
//                            `TMyBinDispCtrl *BinDisCtrl; // opaque; NULL
//                            until UI wave wires InstallColorBinDisplay`
//                            (database.h:300, 20260818) -- this is WHY
//                            ChangeBinDispStatus/DoShowBinDigital are Wave B,
//                            not gated-in-place: >90% of each body is this
//                            one pointer.
//
//  WAVE B (SBWB2) UPDATE, 20260819 -- three of the above absence claims are now
//  STALE (landed by later waves than the 20260818 date on each original claim,
//  same failure mode as the tree-wide "wave-agents-stale-absence-claims" gotcha
//  -- re-checked fresh this wave, not trusted from the old banner text above):
//    fBinSel      : `grep -n "class TfBinSel" forms/fBinSel.h` -- 1 hit, REAL
//                   (forms/fBinSel.h:396). ShowBinSel() below uses
//                   `fBinSel->chkShow0Xbin->Checked` for real, not gated.
//    fAGV         : `grep -n "IsATK_AMR" forms/fAGV.h` -- REAL (delegates to
//                   Automation/AGV_predicates.cpp, W5-Automation-Integrate).
//                   ShowBinSel() below calls `fAGV->IsATK_AMR()` for real.
//    fCounterClear: `grep -n "class TfCounterClear" forms/fCounterClear.h` --
//                   REAL (landed FW-SecCC, 20260819) -- NOT exercised by
//                   ShowBinSel (this wave's target never calls ClearCount),
//                   so GATE (B1) in ShowCategoryBin above is untouched by this
//                   wave -- flagged here so it is not mistaken for still-
//                   absent by a future reader of this file.
//  NEW absence this wave: fSortCT->myCountPanel[i].pnlYield/.pnlCount -- `grep
//  -n "myCountPanel" forms/fSortCT.h` -- 0 hits, 20260819 (fSortCT itself is
//  real; this ONE member array is not on its facade). See GATE (B9).
// =============================================================================
#include "forms/fShowBinSelect.h"
#include "forms/fSecurity.h"   // AI(W906-FW-SecUnlock) 20260819: fSecurity->Insufficient (B6 permission half dissolved)
#include "forms/fCounterClear.h"   // AI(W906-FW-UNGATE-W29) 20260826: fCounterClear (:214) + ClearCount (:194) -- GATE (B1) opened. cCounterClear.cpp 與本檔同屬 ht9045_sm (CMakeLists.txt:1888/:1895)，無新 link edge。
#include "forms/fBinSel.h"    // AI(W906-FW-SBWB2) 20260819: fBinSel->chkShow0Xbin (ShowBinSel, real -- see banner UPDATE below)
#include "SECSGEM/SecsEventType.h"     // AI(W906-FW-SBWB2) 20260819: SECS_EVENT.OutputPort1BinCode (ShowBinSel)
#include "SECSGEM/SecsEventReport.h"   // AI(W906-FW-SBWB2) 20260819: EventReport() (ShowBinSel)

#include "MachineType.h"           // enums, CC_* customer codes, ChangeToPercentage<T>/ChangeToFloat<T>
#include "cmydef.h"                  // SystemStart/iHome/InitialOK/iTestBinCount/iByBinCnt[]/
                                       // iSV_ErrBinCnt/iSVByBinCount[]/AccessLevel/iDefHonPrecLevel/
                                       // StipulateInputCount/iNetUPH/iGrossUPH/iPauseTime/bRefreshFunction
#include "cprod.h"                    // Prod/TestIF/TestIF_File/RunInfo/BinSelect/TastCategory
#include "LastSet.h"                   // LastSet
#include "Config.h"                     // IniConfig
#include "CosFunction.h"                 // CosFunction
#include "aHotPlateSubstrate.h"           // TestSocket (TMyKitSuck header choice -- see KNOWLEDGE.md)
#include "cSocket.h"                       // TArm/TMySocket, ArmData[3]/ArmHistory[3]
#include "cinitial.h"                       // IsNNMode()/NN_1Row/NN_2Row
#include "csystem.h"                          // ShuttleHasIC()/IndexHasIC()
#include "common.h"                          // MyTickCount()
#include "cMyDB.h"                            // MyDBIProcess
#include "atester_shims.h"                     // fContact (TfContactShim)
#include "atester_ProcessCount.h"                // DoLowYieldAlarm
#include "AutoClean/AutoClean.h"                  // InitialAutoCleanAllTask
#include "FormsFacade.h"                           // fMain
#include "forms/fYieldMonitoring.h"                 // fYieldMonitoring
#include "forms/fHome.h"                            // AI(W906-FW3-SBS-WD) 20260820: fHome->fShow (ChangeBinDispStatus)

// AI(W906-FW3-SBS-WD) 20260820: ChangeBinDispStatus/DoShowBinDigital need
// HSys.BinDisCtrl's now-CONCRETE type (BinDisplay/MyBinDisp.h landed this
// same day, AI(W906-BinDisp-WA)) -- database.h alone only forward-declares
// `class TMyBinDispCtrl;` (database.h:63), which is enough for a pointer
// field but not for calling any of its methods. Both direct derefs stay
// GATEd (see WAVE D SCOPE / GATE REGISTER, forms/fShowBinSelect.h) because
// HSys.BinDisCtrl is permanently NULL in this port regardless of this
// header's presence -- it is included purely so the GATEd comments can cite
// real method signatures and so any future un-gating has the type on hand.
#include "database.h"              // HSys / SYSTEM_MODULAR::BinDisCtrl
#include "BinDisplay/MyBinDisp.h"  // TMyBinDispCtrl (concrete, W906-BinDisp-WA)
#include "myswitch.h"              // SW[] / TMySwitch (DoShowBinDigital's digital-panel switch machine)
#include "myTimer.h"               // TQPF_Timer (AlarmDelay/BinTimer file-scope globals, matching golden)

#include <cstdlib>   // atoi
#include <algorithm> // (kept for parity; no direct use this wave)

// AI(W906-FW-SBWB2) 20260819: golden Graphics.hpp clGray -- this tree has no
// shared Graphics-compat header (confirmed this wave: `grep -rn "const TColor
// clGray" --include=*.h .` finds it defined LOCALLY, once per consuming TU, in
// ATC/ATCInterface.h, EJ1N/MyOmronPanel.h and VacuumUnit/MyVacuumPanel.h, each
// with the identical value below and each with its own "this file adds its own
// const TColor clGray locally" note) -- ShowBinSel is this TU's first user of
// the name, so it gets its own copy, same established idiom.
const TColor clGray = TColor(0x00808080);   // golden Graphics.hpp clGray

// AI(W906-FW-SBWC) 20260819: golden Graphics.hpp clGreen/clBtnFace -- same
// "each consuming TU defines it locally" idiom as clGray above (confirmed
// this wave: `grep -rn "const TColor clGreen" --include=*.h .` finds it
// LOCALLY defined, identical value, in mykitsuck.h/VacuumUnit/MyVacuumPanel.h
// via `using vclcompat::clGreen;`, and as its own const in vclcompat/
// TrayCore.h/LedCore.h/BtnPanelCore.h -- ShowBinSel_ARTNor/ShowBinSel_ARTRT
// are this TU's first users of either name).
const TColor clGreen   = TColor(0x00008000);   // golden Graphics.hpp clGreen
const TColor clBtnFace = TColor(0x8000000F);   // golden Graphics.hpp clBtnFace (COLOR_BTNFACE)

// AI(W906-FW3-SBS-WD) 20260820: golden Graphics.hpp clRed/clBlack -- same
// "each consuming TU defines it locally" idiom as clGray/clGreen above
// (confirmed this wave: `grep -rn "const TColor clRed" vclcompat/` finds it
// LOCALLY defined, identical value 0x000000FF, in vclcompat/BtnPanelCore.h/
// LedCore.h/TrayCore.h; clBlack likewise 0x00000000) -- ChangeBinDispStatus
// is this TU's first user of either name (its local `ColorMap[]` literal).
const TColor clRed   = TColor(0x000000FF);   // golden Graphics.hpp clRed
const TColor clBlack = TColor(0x00000000);   // golden Graphics.hpp clBlack

//---------------------------------------------------------------------------
// AI(W906-FW-YEnable) 20260818: homecoming -- the live global is now backed
// by a real instance, BUT this ctor is NOT the same "pure field bootstrap"
// shape as cContactCT.cpp's TfContactCT (re-verified this wave, per task
// brief, by reading the full chain): the ctor calls ShowInitialString(),
// whose own tail statement unconditionally calls ShowCategoryBin(), which
// dereferences `fContact->fShow` (golden :1729) and, on the common
// `LastSet.iTester != _2D_SORT` path, `ArmData[0]/ArmData[1]->GetSelBin()`
// (golden :1802 onward, unconditional inside that path -- not gated behind
// any flag). `fContact` (atester_shims.cpp) and `ArmData[0..2]`
// (cSocket.cpp's own file-scope `ArmDataBootstrap`, see that file's banner)
// are BOTH raw pointers populated by a DIFFERENT translation unit's dynamic
// initializer -- C++ gives no cross-TU dynamic-initialization-order
// guarantee (the exact invariant cSocket.cpp's ArmDataBootstrap banner
// documents: "read from ordinary runtime functions and NEVER from another
// TU's static initialiser"). Unlike golden, where the VCL form is
// constructed inside WinMain strictly after every global already exists,
// this port's `fShowBinSelect` is a plain static-storage global whose own
// dynamic initializer could in principle run before either of those two --
// see the SIOF guard inside ShowInitialString() (this file) added
// specifically for this homecoming, which makes this constructor safe
// regardless of link/init order.
TfShowBinSelect *fShowBinSelect = new TfShowBinSelect();

//---------------------------------------------------------------------------
//  DelDot (free function) -- golden :178-205
//---------------------------------------------------------------------------
AnsiString DelDot(AnsiString asBuffer)
{
    AnsiString asReturn;
    int iBegin = 0;
    for (int i = 0; i < iTestBinCount; i++)   // kevin 20140317: 256 bin
    {
        if (i == 1)
        {
            iBegin = asBuffer.Pos(AnsiString(i) + " ");
            if (iBegin != 1)
                continue;
        }
        else
            iBegin = asBuffer.Pos(" " + AnsiString(i) + " ");
        if (iBegin > 0)
        {
            break;
        }
    }

    if (iBegin == 0)
    {
        iBegin = asBuffer.Pos(AnsiString("E"));
    }

    asReturn = asBuffer.Delete(1, iBegin - 1);
    return asReturn;
}

//---------------------------------------------------------------------------
//  TfShowBinSelect::TfShowBinSelect -- DEVIATION bootstrap ctor.
//  See forms/fShowBinSelect.h CTOR NOTE for why this is NOT a verbatim
//  translation of golden's ctor body (golden :44-161).
//---------------------------------------------------------------------------
TfShowBinSelect::TfShowBinSelect()
{
    for (int i = 0; i < e3TrayCount; i++)
    {
        MyBinSel[i]         = new TfShowBinSelectLabel();   // AI(W906-FW-SBWB2) 20260819: Font->Color (ShowBinSel)
        MyBinSelLab[i]      = new TLabel();
        MyBinSelARTFT[i]    = new TfShowBinSelectLabel();   // AI(W906-FW-SBWC) 20260819: Font->Color (ShowBinSel_ARTNor)
        MyBinSelARTFTLab[i] = new TLabel();
        MyBinSelARTRT[i]    = new TfShowBinSelectLabel();   // AI(W906-FW-SBWC) 20260819: Font->Color (ShowBinSel_ARTRT)
        MyBinSelARTRTLab[i] = new TLabel();
        grpBinDisp[i]       = new TGroupBox();
        // AI(W906-FW-SBWB2) 20260819: UnLoadPanel[]/UnLoadLabel[] --
        // see forms/fShowBinSelect.h member-declaration comment for why
        // these are member arrays, not ~33 individually-named pointers.
        UnLoadPanel[i] = new TPanel();
        UnLoadLabel[i] = new TfShowBinSelectLabel();   // AI(W906-FW-SBWB2) 20260819: Font->Color (ShowBinSel)
    }
    for (int i = 0; i < eTrayCount; i++)
    {
        EditAi[i] = new TLabeledEdit();
        // GATE (cosmetic): tempAi[i]->Align=alTop; -- golden :132. No Align
        // field exists on vclcompat::TLabeledEdit and nothing reads it back.
    }

    ColorRed = 1;      // 1:red, 2:green, 3:orange
    ColorGreen = 2;
    ColorOrange = 3;

    bShow = false;
    tsUPH = new TStringList();

    for (int i = 0; i < 20; i++)   // Steven 20140621: For SECS GEM
    {
        tsUPH->Add("");
    }

    // AI(W906-FW-YEnable) 20260818: SIOF guard #2 (measured, not speculative) --
    // this ctor now runs as a static initializer in EVERY god-stack exe (the
    // dissolved (Y2) gates in uYieldMonitoring.cpp make the linker pull this
    // TU everywhere), and the first FW-YEnable gate went 64/139 red on it in
    // two link-order-dependent shapes (gdb backtraces, build_fwyeng):
    //   (a) SEGV -- sBinCode_ATK[] lives in cmydef.cpp; assigning another
    //       TU's not-yet-constructed AnsiString reaches std::string::
    //       _M_replace through a null _M_p (test_cUnitConvert).
    //   (b) out_of_range -- when the bootstrap TUs happened to init first,
    //       the tail guard inside ShowInitialString() PASSED and
    //       ShowCategoryBin() indexed default-sized TStringGrid Cells with
    //       config-dependent dimensions still zero (test_contactct_core).
    // golden runs this ctor inside WinMain, strictly after every global AND
    // after config load; INIFileGeneral != 0 reproduces exactly that
    // precondition (nullptr constant-init, set only by LoadMachineConfig,
    // common.cpp:1459 -- the same sentinel cObserver.cpp's ctor already
    // uses). Skipping the priming at static init is self-healing: every
    // dissolved (Y2) call site plus uHGemHT9045.cpp's SECS handlers re-run
    // ShowCategoryBin() from the runtime loop, strictly after static init.
    if (INIFileGeneral != 0)
    {
        for (int i = 0; i < eTrayCount; i++)
        {
            sBinCode_ATK[i] = "";
        }

        ShowInitialString();
    }

    if (SPIL_FOR_QLE == 1)   // JerryYang 20251020: partial device-ejection feature
    {
        palAutoDeviceEjection->Visible = true;
        palAutoDeviceEjection->Top = 230;
    }
    else
    {
        palAutoDeviceEjection->Visible = false;
    }

    // GATE (cosmetic): btReturn->Align=alBottom; -- golden :160. No Align
    // field exists on vclcompat::TButton and nothing reads it back.

    bUpdateBinDigital = false;
    iShowBinDigitalTask = 0;
    iLowYieldBinSelectContactCount = 0;
    Left = 0; Top = 0; Width = 0; Height = 0;
}

//---------------------------------------------------------------------------
//  FormDestroy -- golden :163-176
//---------------------------------------------------------------------------
void TfShowBinSelect::FormDestroy(TObject * /*Sender*/)
{
    try
    {
        TimerAutoCleanCount->Enabled = false;   // Steven 20160727
        tsUPH->Clear();                          // Ifor 20170603 (wei): clear TStringList before deleting
        delete tsUPH;                            // Steven 20160108: release memory
    }
    catch (...)
    {
        // NOTE: explicit 3-arg form disambiguates two overlapping
        // MyDBIProcess declarations both visible in this TU (same
        // pre-existing tree-wide overload shape as cContactCT.cpp's
        // identical FormDestroy fix -- see that file's note).
        MyDBIProcess("Exception", "TfShowBinSelect::FormDestroy", "");
    }
    LogSoftwareOffTime("TfShowBinSelect, FormDestroy");   // Steven 20210526: record software run time
}

//---------------------------------------------------------------------------
//  FormClose -- golden :868-873
//---------------------------------------------------------------------------
void TfShowBinSelect::FormClose(TObject * /*Sender*/)
{
    bShow = false;
}

//---------------------------------------------------------------------------
//  InitShowBinDigital -- golden :874-878
//---------------------------------------------------------------------------
void TfShowBinSelect::InitShowBinDigital()
{
    iShowBinDigitalTask = 1;
    bUpdateBinDigital = true;
}

//---------------------------------------------------------------------------
//  SetLabelVisible -- golden :1425-1434
//---------------------------------------------------------------------------
void TfShowBinSelect::SetLabelVisible(int iTag, bool bVisible)
{
    MyBinSel[iTag]->Visible         = bVisible;
    MyBinSelLab[iTag]->Visible      = bVisible;
    MyBinSelARTFT[iTag]->Visible    = bVisible;
    MyBinSelARTFTLab[iTag]->Visible = bVisible;
    MyBinSelARTRT[iTag]->Visible    = bVisible;
    MyBinSelARTRTLab[iTag]->Visible = bVisible;
    grpBinDisp[iTag]->Visible       = bVisible;
}

//---------------------------------------------------------------------------
//  SetAutoVisible -- golden :1436-1477
//---------------------------------------------------------------------------
void TfShowBinSelect::SetAutoVisible()
{
    gbBinBox->Visible   = (iHWFix_BinBox == 1);
    pnlMag123->Visible  = (AUTO3_IS_MAGAZINE > 0);
    pnlFix789->Visible  = (AUTO_EMPTY_COLOR >= 3);
    pnlAuto456->Visible = (AUTO_EMPTY_COLOR >= 3);
    gbAuto6->Visible    = (AUTO_EMPTY_COLOR >= 4);

    for (int i = eAuto1; i < eTrayCount; i++)
    {
        if (Prod.iTrayType[i] == tNotUse)
        {
            SetLabelVisible(i, false);
        }
        else
        {
            SetLabelVisible(i, true);
        }
    }

    if (AUTO3_IS_MAGAZINE > 0)
        SetLabelVisible(iMagAtAuto, false);

    if (AUTO_EMPTY_COLOR >= 3)   // Steven 20230907: For HT-9011UC
    {
        if (CosFunction.bUseTrayUpDownSet)   // wei 20160224: TSMC fix up/down
        {
            pnlFix789->Visible = true;
        }
        else
        {
            pnlFix789->Visible = false;
        }
    }

    if (iHWFix_BinBox == 1)   // kevin 20160819: error bin box
    {
        MyBinSel[eBulkBox]->Caption      = "E";
        MyBinSelARTFT[eBulkBox]->Caption = "E";
        MyBinSelARTRT[eBulkBox]->Caption = "E";
    }
}

//---------------------------------------------------------------------------
//  ShowInitialString -- golden :1634-1700
//---------------------------------------------------------------------------
void TfShowBinSelect::ShowInitialString()
{
    if (CosFunction.bSortingBy2DList == true &&
        LastSet.iTester == _2D_SORT &&
        TestIF_File.bSortingBy2DIDList == true)   // JerryYang 20230322: 2DID expect count
    {
        StrGrdCategory->ColCount = 4;
        StrGrdCategory->Cells[1][0] = "expect count";
        StrGrdCategory->Cells[2][0] = "Count";
        StrGrdCategory->Cells[3][0] = "Percent";
    }
    else
    {
        StrGrdCategory->Cells[1][0] = "Count";
        StrGrdCategory->Cells[2][0] = "Percent";
    }
    StrGrdCategory->RowCount = iTestBinCount + 2;

    if (LastSet.iTester == _2D_SORT)   // Frank 20221122: 2DID sorting for ATK
    {
        for (int i = 0; i < iTestBinCount; i++)   // JerryYang 20220425: Nvidia Ryan requested "Bin" wording
            StrGrdCategory->Cells[0][1 + i] = "BIN " + AnsiString(i);
    }
    else
    {
        for (int i = 0; i < iTestBinCount; i++)
            StrGrdCategory->Cells[0][1 + i] = "Category " + AnsiString(i);
    }

    StrGrdCategory->Cells[0][iTestBinCount + 1] = "Error Bin";

    if (CosFunction.bCategoryInfoByContactCT)   // Sam 20240131: new CategoryInfo record set using ContactCT
    {
        StrGrdCategoryContCT->Cells[1][0] = "Count";
        StrGrdCategoryContCT->Cells[2][0] = "Percent";
        StrGrdCategoryContCT->RowCount = iTestBinCount + 2;
        for (int i = 0; i < iTestBinCount; i++)
            StrGrdCategoryContCT->Cells[0][1 + i] = "Category " + AnsiString(i);
        StrGrdCategoryContCT->Cells[0][iTestBinCount + 1] = "Error Bin";
    }

    if (USE_AUTO_RETEST == eartInstall || CosFunction.bUseARTSortCount)   // wei 20150923: add ART count
                                                                             // Ifor 20170316 (wei): add MRT Mode
    {
        StrGrdCategoryART->Cells[1][0] = "Count";
        StrGrdCategoryART->Cells[2][0] = "Percent";
        StrGrdCategoryART->RowCount = iTestBinCount + 2;
        for (int i = 0; i < iTestBinCount; i++)
            StrGrdCategoryART->Cells[0][1 + i] = "Category " + AnsiString(i);

        StrGrdCategoryART->Cells[0][iTestBinCount + 1] = "Error Bin";

        StrARTSkipICCount->Cells[1][0] = "Count";   // Frank 20160819
        StrARTSkipICCount->RowCount = 22;
        StrARTSkipICCount->ColCount = 2;

        for (int i = 0; i < 20; i++)
        {
            if (i == 0)
                StrARTSkipICCount->Cells[0][1 + i] = "FT " + AnsiString(1);
            else
                StrARTSkipICCount->Cells[0][1 + i] = "RT " + AnsiString(i);
        }
        StrARTSkipICCount->Cells[0][21] = "Total";
    }

    // AI(W906-FW-YEnable) 20260818: SIOF guard, NOT a golden deviation --
    // PORT-ONLY safety net for the new static-init `fShowBinSelect` global
    // (see this file's homecoming note on that definition for the full
    // reasoning). golden's ctor chain reaches this exact point strictly
    // after WinMain has already brought up every other global, so golden
    // never needed this check. `fContact` and `ArmData[0]/ArmData[1]` are
    // each a raw pointer owned by a DIFFERENT translation unit's own dynamic
    // initializer (atester_shims.cpp / cSocket.cpp respectively); cross-TU
    // dynamic-init order is unspecified, so if this ctor's call happens to
    // run first, `ShowCategoryBin()` would dereference a still-null pointer
    // (`fContact->fShow` at golden :1729 unconditionally, `ArmData[0]/
    // ArmData[1]->GetSelBin()` at golden :1802 on the common
    // `LastSet.iTester != _2D_SORT` path -- verified this wave by reading
    // ShowCategoryBin()'s full body). Skipping this ONE priming call when
    // either isn't bootstrapped yet is self-healing, not a behaviour change:
    // every dissolved fYieldMonitoring (Y2) gate call site
    // (uYieldMonitoring.cpp, this wave) plus uHGemHT9045.cpp's SECS handlers
    // call ShowCategoryBin() again on the next real tick, all from main()'s
    // own runtime loop -- strictly after every TU's static initialization
    // has finished. Every statement ABOVE this guard (the grid header/
    // RowCount setup) touches no cross-TU pointer and is unaffected either
    // way.
    if (fContact != 0 && ArmData[0] != 0 && ArmData[1] != 0)
        ShowCategoryBin();
}

//---------------------------------------------------------------------------
//  ShowCategoryBin -- golden :1701-2052
//  GATE (B1): fCounterClear->ClearCount(ctBinCount) x3, see forms/
//  fShowBinSelect.h GATE REGISTER.
//---------------------------------------------------------------------------
void TfShowBinSelect::ShowCategoryBin()
{
    int i = 0, sum = 0, Sum_ART = 0, iTemp = 0, Sum_ConCT = 0;   // Sam 20240131: ContCT CategoryInfo
    double f = 0.0, fSPBin = 0.0;                                 // Isaac 20171113 (Steven): ATK Special Bin Yield alarm
    AnsiString S1, S2;
    (void)S2;   // golden local, never assigned/read in this function body (verified this wave) -- kept verbatim
    static bool bFirstTime = true;

    if (bFirstTime == true)
    {
        bFirstTime = false;
        for (i = 0; i < TEST_MAX_BIN; i++)
        {
            iLoadPersentCT[i] = 0;
            iLoadCountCT[i] = 0;
            iYeildCT[i] = 0;   // wei 20151111
        }
    }

    if (CUSTOMER_CODE == CC_ASE_CL)
    {
    }
    else
    {
        iSV_ErrBinCnt = LastSet.iBinData32[0][iTestBinCount];
        for (i = 0; i < TEST_MAX_BIN; i++)
            iSVByBinCount[i] = LastSet.iBinData32[0][i];
    }

    if (fContact->fShow == false)
    {
        if (CosFunction.bLowYieldUseContactCounts)   // Sam 20230620: optimise Smart Auto Clean
        {
            for (i = 0; i < iTestBinCount; i++)
                sum += LastSet.iBinData32[0][i];
        }
        else
        {
            for (i = 0; i < eTrayCount; i++)
            {
                sum     += LastSet.BinCT[0][iTo3Unload[i]];
                Sum_ART += LastSet.BinCT_ART[0][iTo3Unload[i]];   // wei 20150923: add ART count
            }
        }

        if (CosFunction.bCategoryInfoByContactCT)   // Sam 20240131: ContCT CategoryInfo
            Sum_ConCT = ArmData[0]->GetTotalCT() + ArmData[1]->GetTotalCT();

        if (LastSet.iTester == _2D_SORT)   // Frank 20221122: 2DID sorting for ATK
        {
            for (i = 0; i < iTestBinCount + 1; i++)   // Steven 20121112: RS232 supports 32Bin
            {
                fShowBinSelect->StrGrdCategory->Cells[0][1 + i] = "";
                fShowBinSelect->StrGrdCategory->Cells[1][1 + i] = "";
                fShowBinSelect->StrGrdCategory->Cells[2][1 + i] = "";
                fShowBinSelect->StrGrdCategory->Cells[3][1 + i] = "";
                if (iByBinCnt[i] > 0 || i == iTestBinCount)
                {
                    if (i == iTestBinCount)
                    {
                        fShowBinSelect->StrGrdCategory->Cells[0][1 + iTemp] = "Error Bin";
                    }
                    else
                    {
                        fShowBinSelect->StrGrdCategory->Cells[0][1 + iTemp] = "BIN " + AnsiString(i);
                    }
                    fShowBinSelect->StrGrdCategory->Cells[2][1 + iTemp] = LastSet.iBinData32[0][i];
                    fShowBinSelect->StrGrdCategoryART->Cells[1][1 + iTemp] = LastSet.iBinData32_ART[0][i];   // wei 20150923: add ART count
                    fShowBinSelect->StrGrdCategory->Cells[1][1 + iTemp] = iByBinCnt[i];
                    if (sum > 0)
                    {
                        f = ChangeToFloat((double)LastSet.iBinData32[0][i], (double)sum);   // Steven 20250820: guard against divide-by-0
                        fShowBinSelect->StrGrdCategory->Cells[3][1 + iTemp] = ChangeToPercentage((double)LastSet.iBinData32[0][i], (double)sum);
                    }
                    else
                    {
                        fShowBinSelect->StrGrdCategory->Cells[3][1 + iTemp] = "0.00%";
                    }

                    if ((USE_AUTO_RETEST == eartInstall && IniConfig.bA10_AutoReTest) ||
                        CosFunction.bUseARTSortCount)   // wei 20150923: add ART count / Ifor 20170316 (wei): add MRT Mode
                    {
                        if (Sum_ART > 0)
                        {
                            fShowBinSelect->StrGrdCategoryART->Cells[2][1 + iTemp] = ChangeToPercentage((double)LastSet.iBinData32_ART[0][i], (double)Sum_ART);
                        }
                        else
                        {
                            fShowBinSelect->StrGrdCategoryART->Cells[2][1 + iTemp] = "0.00%";
                        }
                    }
                    iTemp++;
                }
            }
        }
        else
        {
            for (i = 0; i < iTestBinCount + 1; i++)   // Steven 20121112: RS232 supports 32Bin
            {
                StrGrdCategory->Cells[1][1 + i] = LastSet.iBinData32[0][i];
                StrGrdCategoryART->Cells[1][1 + i] = LastSet.iBinData32_ART[0][i];   // wei 20150923: add ART count
                StrGrdCategoryContCT->Cells[1][1 + i] = (unsigned int)(ArmData[0]->GetSelBin(i) + ArmData[1]->GetSelBin(i));   // Sam 20240131 -- cast disambiguates AnsiString::operator= (unsigned long)

                if (sum > 0)
                {
                    f = ChangeToFloat((double)LastSet.iBinData32[0][i], (double)sum);
                    StrGrdCategory->Cells[2][1 + i] = ChangeToPercentage((double)LastSet.iBinData32[0][i], (double)sum);
                }
                else
                {
                    StrGrdCategory->Cells[2][1 + i] = "0.00%";
                }

                if (CosFunction.bCategoryInfoByContactCT)
                {
                    if (Sum_ConCT > 0)
                    {
                        StrGrdCategoryContCT->Cells[2][1 + i] = ChangeToPercentage((double)(ArmData[0]->GetSelBin(i) + ArmData[1]->GetSelBin(i)), (double)Sum_ConCT);
                    }
                    else
                    {
                        StrGrdCategoryContCT->Cells[2][1 + i] = "0.00%";
                    }
                }

                if ((USE_AUTO_RETEST == eartInstall && IniConfig.bA10_AutoReTest) ||
                    CosFunction.bUseARTSortCount)   // wei 20150923: add ART count / Ifor 20170316 (wei): add MRT Mode
                {
                    if (Sum_ART > 0)
                    {
                        StrGrdCategoryART->Cells[2][1 + i] = ChangeToPercentage((double)LastSet.iBinData32_ART[0][i], (double)Sum_ART);
                    }
                    else
                    {
                        StrGrdCategoryART->Cells[2][1 + i] = "0.00%";
                    }
                }
            }

            if (LastSet.iTester != _2D_SORT)   // JerryYang 20230322: 2DID sort doesn't need this
            {
                for (i = 0; i < iTestBinCount; i++)   // Steven 20121112: RS232 supports 32Bin
                {
                    if (RunInfo.iUnloadCount < iLoadPersentCT[i])   // Steven 20140905: LastSet.SendCT[0] -> RunInfo.iUnloadCount
                        iLoadPersentCT[i] = RunInfo.iUnloadCount;

                    if (RunInfo.iUnloadCount < iLoadCountCT[i])
                        iLoadCountCT[i] = RunInfo.iUnloadCount;

                    if (RunInfo.iUnloadCount < iYeildCT[i])   // wei 20151111
                        iYeildCT[i] = RunInfo.iUnloadCount;

                    StrGrdCategory->Cells[1][1 + i] = LastSet.iBinData32[0][i];
                    StrGrdCategoryART->Cells[1][1 + i] = LastSet.iBinData32_ART[0][i];   // wei 20150923: add ART count
                    StrGrdCategoryContCT->Cells[1][1 + i] = (unsigned int)(ArmData[0]->GetSelBin(i) + ArmData[1]->GetSelBin(i));   // Sam 20240131 -- cast disambiguates AnsiString::operator= (unsigned long)

                    if (sum > 0)
                    {
                        f = ChangeToFloat((double)LastSet.iBinData32[0][i], (double)sum);
                        fSPBin = ChangeToFloat((double)LastSet.iBinData32[0][Prod.iSCKART_SPBinSelect], (double)sum);   // Isaac 20171113 (Steven): ATK Special Bin Yield alarm

                        StrGrdCategory->Cells[2][1 + i] = ChangeToPercentage((double)LastSet.iBinData32[0][i], (double)sum);

                        if (CosFunction.bCategoryInfoByContactCT)   // Sam 20240131
                        {
                            if (Sum_ConCT > 0)
                            {
                                f = ChangeToFloat((double)(ArmData[0]->GetSelBin(i) + ArmData[1]->GetSelBin(i)), (double)Sum_ConCT);
                                StrGrdCategoryContCT->Cells[2][1 + i] = ChangeToPercentage((double)(ArmData[0]->GetSelBin(i) + ArmData[1]->GetSelBin(i)), (double)Sum_ConCT);
                            }
                            else
                            {
                                f = 0.0;
                            }
                        }

                        if ((USE_AUTO_RETEST == eartInstall && IniConfig.bA10_AutoReTest) ||
                            CosFunction.bUseARTSortCount)   // wei 20150923: add ART count / Ifor 20170316 (wei): add MRT Mode
                        {
                            StrGrdCategoryART->Cells[2][1 + i] = ChangeToPercentage((double)LastSet.iBinData32_ART[0][i], (double)Sum_ART);
                        }

                        if (TestIF_File.bSCKART_EnableSPBinAlarm == true &&
                            TestIF_File.bSCKART_EnableART == true)   // Isaac 20171113 (Steven): ATK Special Bin Yield alarm
                        {
                            if (Prod.dSCKART_SPBinAlarmYield != 0.0 && fSPBin >= Prod.dSCKART_SPBinAlarmYield)
                            {
                                bSPBinYieldAlarm = true;
                            }
                            else
                            {
                                bSPBinYieldAlarm = false;
                            }
                        }
                        else
                        {
                            bSPBinYieldAlarm = false;
                        }

                        if (SystemStart && iHome == 0)
                        {
                            if (Prod.bFailure[i] == true)
                            {
                                if (IniConfig.bI30ContFailBin)   // kevin 20160407: add Fail-bin-over-limit warning
                                {
                                    if (Prod.dFailureLimit[i] != 0.0 && f >= Prod.dFailureLimit[i])
                                    {
                                        fYieldMonitoring->ClearYieldCount();

                                        if (CUSTOMER_CODE != CC_KYEC_LEE)   // wei 20151111
                                            iLoadPersentCT[i] = RunInfo.iUnloadCount;

                                        if (IniConfig.bEnableAutoCleanFunction &&   // JerryYang 20161121: fix Yield alarm needing to
                                                                                      // finish ONE CYCLE first, affecting auto clean
                                            TestIF.iAutoClean_Function == true &&
                                            (TestIF.iAutoClean_Mode & M_SOCKET_ALARM))
                                        {
                                            InitialAutoCleanAllTask();   // Sam 20230504: tidy InitialAutoCleanTask
                                        }
                                        S1.sprintf("Category %d count over limit %2.1f%% ", i, Prod.dFailureLimit[i]);
                                        DoLowYieldAlarm("WAR07357", S1);
                                        break;
                                    }
                                }
                                else
                                {
                                    if (CosFunction.bYieldControlBinSelectUseContactCount == true)   // KaiChen 20181115:
                                                                                                         // BinSelect Yield control uses Contact Count
                                    {
                                        if (iLowYieldBinSelectContactCount > Prod.iPersentIgnore[i] &&
                                            Prod.dFailureLimit[i] != 0 &&
                                            f >= Prod.dFailureLimit[i])
                                        {
                                            fShowBinSelect->iLowYieldBinSelectContactCount = 0;   // KaiChen 20181115
                                            fYieldMonitoring->ClearYieldCount();

                                            if (CUSTOMER_CODE != CC_KYEC_LEE)   // wei 20151111
                                                iLoadPersentCT[i] = RunInfo.iUnloadCount;
                                            if (IniConfig.bEnableAutoCleanFunction && TestIF.iAutoClean_Function == true && (TestIF.iAutoClean_Mode & M_SOCKET_ALARM))
                                            {
                                                InitialAutoCleanAllTask();   // Sam 20230504: tidy InitialAutoCleanTask
                                            }
                                            S1.sprintf("Category %d count over limit %2.1f%% ", i, Prod.dFailureLimit[i]);
                                            DoLowYieldAlarm("WAR07357", S1);   // Steven 20180627 (wei): consolidate Low Yield Alarm
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        bool bNeedCheck = false;
                                        if (CosFunction.bLowYieldUseContactCounts &&   // Sam 20230620: optimise Smart Auto Clean
                                            CUSTOMER_CODE != CC_PTI)                    // Sam 20250102: fix PTI AI Clean
                                        {
                                            if (Sum_ConCT > Prod.iPersentIgnore[i])   // Sam 20250102: fix PTI AI Clean
                                                bNeedCheck = true;
                                        }
                                        else
                                        {
                                            if (RunInfo.iUnloadCount - iLoadPersentCT[i] > Prod.iPersentIgnore[i])   // Steven 20140905
                                                bNeedCheck = true;
                                        }

                                        if (bNeedCheck &&
                                            Prod.dFailureLimit[i] != 0 &&
                                            f >= Prod.dFailureLimit[i])
                                        {
                                            fYieldMonitoring->ClearYieldCount();

                                            if (CUSTOMER_CODE != CC_KYEC_LEE)   // wei 20151111
                                                iLoadPersentCT[i] = RunInfo.iUnloadCount;
                                            if (IniConfig.bEnableAutoCleanFunction && TestIF.iAutoClean_Function == true && (TestIF.iAutoClean_Mode & M_SOCKET_ALARM))
                                            {
                                                InitialAutoCleanAllTask();   // Sam 20230504: tidy InitialAutoCleanTask
                                            }
                                            S1.sprintf("Category %d count over limit %2.1f%% ", i, Prod.dFailureLimit[i]);
                                            DoLowYieldAlarm("WAR07357", S1);   // Steven 20180627 (wei): consolidate Low Yield Alarm
                                            if (CosFunction.bSmartAutoClean)
                                            {
                                                // Smart Auto Clean clears the data once it finishes running
                                            }
                                            else
                                            {
                                                // GATE (B1) OPENED 20260826 (FW-UNGATE-W29): golden :1983.
                                                // 舊理由「fCounterClear has NO facade anywhere in the tree」
                                                // （標 20260818）已於 20260819 死於 wave FW-SecCC：
                                                // forms/fCounterClear.h:166 class TfCounterClear、:194
                                                // ClearCount(int)、cCounterClear.cpp:26 無條件全域。
                                                // 同一呼叫已在 csystem.cpp:11425 未 gate 地跑著。
                                                fCounterClear->ClearCount(ctBinCount);
                                            }
                                            break;
                                        }
                                    }
                                }
                            }
                            else if (Prod.bFailCountEnable[i] == true)
                            {
                                if (IniConfig.bI30ContFailBin)   // kevin 20160407: add Fail-bin-over-limit warning
                                {
                                    if (Prod.iFailCountLimit[i] != 0 && LastSet.iBinData32[0][i] >= Prod.iFailCountLimit[i])
                                    {
                                        fYieldMonitoring->ClearYieldCount();
                                        if (CUSTOMER_CODE != CC_KYEC_LEE)   // wei 20151111
                                            iLoadCountCT[i] = RunInfo.iUnloadCount;
                                        // GATE (B1) OPENED 20260826 (FW-UNGATE-W29) -- golden :1999.
                                        fCounterClear->ClearCount(ctBinCount);
                                        if (IniConfig.bEnableAutoCleanFunction && TestIF.iAutoClean_Function == true && (TestIF.iAutoClean_Mode & M_SOCKET_ALARM))
                                        {
                                            InitialAutoCleanAllTask();   // Sam 20230504: tidy InitialAutoCleanTask
                                        }
                                        S1.sprintf("BIN %d over limit count %d ", i, Prod.iFailCountLimit[i]);
                                        DoLowYieldAlarm("WAR07358", S1);   // Steven 20180627 (wei): consolidate Low Yield Alarm
                                        break;
                                    }
                                }
                                else
                                {
                                    bool bNeedCheck = false;
                                    if (CosFunction.bLowYieldUseContactCounts &&   // Sam 20230620: optimise Smart Auto Clean
                                        CUSTOMER_CODE != CC_PTI)                    // Sam 20250102: fix PTI AI Clean
                                    {
                                        if (Sum_ConCT > Prod.iFailCountIgnore[i])   // Sam 20250102: fix PTI AI Clean
                                            bNeedCheck = true;
                                    }
                                    else
                                    {
                                        if (RunInfo.iUnloadCount - iLoadCountCT[i] > Prod.iFailCountIgnore[i])   // Steven 20140905
                                            bNeedCheck = true;
                                    }

                                    if (bNeedCheck &&
                                        Prod.iFailCountLimit[i] != 0 &&
                                        LastSet.iBinData32[0][i] >= Prod.iFailCountLimit[i])
                                    {
                                        fYieldMonitoring->ClearYieldCount();
                                        if (CUSTOMER_CODE != CC_KYEC_LEE)   // wei 20151111
                                            iLoadCountCT[i] = RunInfo.iUnloadCount;
                                        S1.sprintf("BIN %d over limit count %d ", i, Prod.iFailCountLimit[i]);
                                        DoLowYieldAlarm("WAR07358", S1);   // Steven 20180627 (wei): consolidate Low Yield Alarm
                                        if (CosFunction.bSmartAutoClean)
                                        {
                                            // Smart Auto Clean clears the data once it finishes running
                                        }
                                        else
                                        {
                                            // GATE (B1) OPENED 20260826 (FW-UNGATE-W29) -- golden :2040.
                                            // 注：golden 在同一個 if 塊的上方 (:2031) 還有一行
                                            // `//fCounterClear->ClearCount(ctBinCount);`，那一行在 golden
                                            // 裡就是被註掉的，本樹不複活它。
                                            fCounterClear->ClearCount(ctBinCount);
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
//  UPH_StringGridDblClick -- golden :2054-2099 (20090720 Steven: Delete 1 UPH Record)
//  GATE (B2): Application->MessageBoxA(...)==IDOK -- see forms/
//  fShowBinSelect.h GATE REGISTER.
//---------------------------------------------------------------------------
void TfShowBinSelect::UPH_StringGridDblClick(TObject * /*Sender*/)
{
    int iRow = 0, iTotalUPH = 0, iCount = 0;
    if (!SystemStart && AccessLevel >= iDefHonPrecLevel &&
        PageControl1->ActivePageIndex == 2)   // jou 2014-06-19: Security has 5 levels, 3 -> iDefHonPrecLevel
    {
        iRow = UPH_StringGrid->Selection.Top;
        if (iRow > 0 && iRow < 11)   // Row is between 1 and 10
        {
            // GATE (B2): fail-closed -- the destructive row-delete loop below
            // does not run without a real confirm dialog.
#if 0
            if (Application->MessageBoxA("Do you want to delete this record?", "Confirm", MB_OKCANCEL) == IDOK)
            {
                for (int i = iRow; i <= 10; i++)   // shift row[i+1]'s value into row[i]
                {
                    UPH_StringGrid->Cells[0][i] = UPH_StringGrid->Cells[0][i + 1];
                    UPH_StringGrid->Cells[1][i] = UPH_StringGrid->Cells[1][i + 1];
                    UPH_StringGrid->Cells[2][i] = UPH_StringGrid->Cells[2][i + 1];
                    UPH_StringGrid->Cells[3][i] = UPH_StringGrid->Cells[3][i + 1];
                    if (IniConfig.bVTESTFunction == true)   // RogerYang 20250224: Weitest requested 3 more columns
                    {
                        UPH_StringGrid->Cells[4][i] = UPH_StringGrid->Cells[4][i + 1];
                        UPH_StringGrid->Cells[5][i] = UPH_StringGrid->Cells[5][i + 1];
                        UPH_StringGrid->Cells[6][i] = UPH_StringGrid->Cells[6][i + 1];
                    }
                }

                for (int i = 1; i <= 10; i++)   // recompute the UPH average
                {
                    try
                    {
                        if (UPH_StringGrid->Cells[3][i] != "")
                        {
                            iTotalUPH += atoi(AnsiString(UPH_StringGrid->Cells[3][i]).c_str());
                            iCount++;
                        }
                    }
                    catch (...)
                    {
                        MyDBIProcess("Exception", "UPH_StringGridDblClick");
                    }
                }
                RunInfo.iAvgUPH = (iCount == 0) ? 0 : iTotalUPH / iCount;
                UPH_StringGrid->Cells[3][12] = RunInfo.iAvgUPH;
            }
#endif
            (void)iTotalUPH; (void)iCount;
        }
    }
}

//---------------------------------------------------------------------------
//  CaculateUPH -- golden :2279-2333 (Steven 20160727: show immediate UPH)
//  GATE (B3): fMain->StatusBar1->Panels->Items[7]->Text=... -- see forms/
//  fShowBinSelect.h GATE REGISTER. The UPH numbers themselves (iNetUPH/
//  iGrossUPH/iRecordEventLogUPH) are computed and stored regardless.
//---------------------------------------------------------------------------
void TfShowBinSelect::CaculateUPH()
{
    static int iOldSum = -1, PassSum = 0;
    static int UPH = 0, UPH2 = 0;
    static DWORD NowTickCount = (DWORD)-1, LastTimeTick = 1, PassSec = 0, PassSec2 = 0;
    int sum = 0;

    for (int i = 0; i < eTrayCount; i++)
    {
        if (Prod.iTrayType[i] != tNotUse)
            sum += LastSet.BinCT[0][iTo3Unload[i]];
    }

    if ((OutArmSuck.HasIC() || ShuttleHasIC() || IndexHasIC()) && sum > iOldSum)
    {
        NowTickCount = MyTickCount();
        if (NowTickCount < LastTimeTick)
        {
            iOldSum = sum;
            LastTimeTick = MyTickCount();
        }
        else
        {
            PassSec = (NowTickCount - LastTimeTick) / 1000;
            PassSec2 = PassSec - (iPauseTime / 1000);
            PassSum = sum - iOldSum;
            if (PassSec > 0)
            {
                UPH = PassSum * 3600 / PassSec;
                iNetUPH = UPH;

                if (PassSec2 > 0)   // Steven 20260421: add zero-guard for PassSec2
                    UPH2 = PassSum * 3600 / PassSec2;
                iGrossUPH = UPH2;
                // GATE (B3): fMain->StatusBar1->Panels->Items[7]->Text=
                // "Net UPH: "+...  /  "Curr UPH: "+... (golden :2313-2320,
                // both CC_ASE_CL and the general customer-code arms).
            }
        }
    }
    else
    {
        iPauseTime = 0;
        iOldSum = sum;
        LastTimeTick = MyTickCount();
        UPH = 0;
    }
    iRecordEventLogUPH = UPH;
}

//---------------------------------------------------------------------------
//  edSLT01Change -- golden :2334-2337
//  GATE (B3): the RHS itself (fMain->StatusBar1->Panels->Items[6]->Text)
//  needs the same missing StatusBar1 -- whole statement gated, no
//  independently computable value.
//---------------------------------------------------------------------------
void TfShowBinSelect::edSLT01Change(TObject * /*Sender*/)
{
    // GATE (B3): strcpy(LastSet.szJamClearData[0], fMain->StatusBar1->
    // Panels->Items[6]->Text.c_str());  // Alick 20160920 add for JCET
}

//---------------------------------------------------------------------------
//  btnSetInpputCntClick -- golden :2339-2352
//---------------------------------------------------------------------------
void TfShowBinSelect::btnSetInpputCntClick(TObject * /*Sender*/)
{
    if (IniConfig.bG15LoadInputCount)   // kevin 20211106: clean out once input count reached
    {
        try
        {
            StipulateInputCount = StrToInt(EdLoadCount->Text.c_str());   // kevin 20211106
        }
        catch (...)
        {
            StipulateInputCount = 0;
        }
    }
}

//---------------------------------------------------------------------------
//  RefreshAiCnt -- golden :2354-2364 (Sam 20210609: Fix AOI software upgrade)
//  GATE (B4): fFixAICCD->UnloadAICntNG[i] -- see forms/fShowBinSelect.h GATE
//  REGISTER. Safe default: blank text, same as the existing tNotUse else-arm.
//---------------------------------------------------------------------------
void TfShowBinSelect::RefreshAiCnt()
{
    for (int i = 0; i < eTrayCount; i++)
    {
        if (Prod.iTrayType[i] != tNotUse)
            EditAi[i]->Text = "";   // GATE (B4): should be IntToStr(fFixAICCD->UnloadAICntNG[i])
        else
            EditAi[i]->Text = "";
    }
}

//---------------------------------------------------------------------------
//  ed_AutoCleanCountClick -- golden :2216-2223
//  GATE (B5) NARROWED 20260826 (FW-UNGATE-W29): fSecurity->Insufficient(43)
//  is LIVE again; only fCleaning->btnResetCleanCountClick stays gated.
//---------------------------------------------------------------------------
void TfShowBinSelect::ed_AutoCleanCountClick(TObject * /*Sender*/)
{
    // AI(W906-FW-UNGATE-W29) 20260826: GATE (B5) 收窄。
    // fSecurity 那半的理由已死：forms/fSecurity.h:561
    // `bool Insufficient(int iType, bool bAlarm = true);` 是真的（本體
    // cSecurity.cpp:650），同一 TU 的 btnClearCountClick 早就在用
    // fSecurity->Insufficient(108)（:1064, FW-SecUnlock 20260819）。
    // 今天的可觀察行為不變：cSecurity.cpp:663-664 在 GATE (SEC1)
    // 沒開的情況下 iMaxLevelItem==0，所以 Insufficient(43) 在 :664
    // 就 return false（連 WAR1676 都不會發），下面整塊不執行。
    // ⚠ 仍然 gated 的只剩 fCleaning->btnResetCleanCountClick(Owner)：
    //   forms/fCleaning.h:24 的 TfCleaning 只有 4 個資料成員
    //   (edCleaningCount/iDeviceCount/bResetCleanCount/b1x2SiteAbClosePutDummy)，
    //   沒有這支方法；且本 facade 也沒有 golden 的 TForm::Owner。
    //   重跑 20260826（掃描器排除 build*/.git/.svn，且會判 #if 0）：
    //   "btnResetCleanCountClick" 全樹共 4 個呼叫站點，**沒有一個是編進去的**：
    //     csystem.cpp:17614  在 #ifdef SOFT_SIMULTE 內（本樹未定義該巨集，
    //                        build/CMakeFiles/*/flags.make 的 CXX_DEFINES 可證）
    //     csystem.cpp:17631  #if 0 GATE G01a（AI(W906-PT-csystem-g2) 20260809）
    //     csystem.cpp:17646  #if 0 GATE G01b（同上）
    //     Command.cpp:17405  #if 0
    //   G01a/G01b 的理由與這裡逐字相同（「NOT a member of the ported
    //   TfCleaning」），golden 本體在 AutoClean/uCleaning.cpp:2095，本樹未翻。
    //   → (SEC1) 一旦開閘，Insufficient(43) 就可能回 true，屆時 clean
    //     count 會只清畫面、沒清實際計數。**這一行必須跟 (SEC1)
    //     同波落地**，不能只開 (SEC1)。
    if (fSecurity->Insufficient(43))                                            // golden :2218
    {
        // GATE (B5-殘留): fCleaning->btnResetCleanCountClick(Owner);   // golden :2220
        ed_AutoCleanCount->Text = 0;                                            // golden :2221
    }
}

//---------------------------------------------------------------------------
//  labAuto1Click -- golden :2225-2242
//  GATE (B7) OPENED 20260826 (FW-UNGATE-W29): fBinSel->chkShow0Xbin->Checked
//  is read for real; the checkbox default is still false, so Width=331 today.
//---------------------------------------------------------------------------
void TfShowBinSelect::labAuto1Click(TObject * /*Sender*/)
{
    fShowBinSelect->Left = 10;
    fShowBinSelect->Top = 130;
    SetAutoVisible();

    if (PageControl1->ActivePageIndex == 0)   // tsTestBin
    {
        fShowBinSelect->Width = 1024;
    }
    else
    {
        // GATE (B7) OPENED 20260826 (FW-UNGATE-W29): 舊理由「fBinSel has NO
        // facade anywhere」（標 20260818）已於 20260819 死於 wave
        // FW-SBWB2：forms/fBinSel.h:588 class TfBinSel、:596
        // TCheckBox *chkShow0Xbin、cBinSel.cpp:158 無條件全域。
        // **同一個 TU 的 :1213 早就在 live 使用 fBinSel->chkShow0Xbin
        // ->Checked**（ShowBinSel），這個 gate 本來就是檔內自相矛盾；
        // forms/fShowBinSelect.h:316-321 自己也已記 STALE。
        // 開閘後：只改 fShowBinSelect->Width（400 vs 331），純 widget
        // 幾何；不寫檔、不動機台、不送對外命令。chkShow0Xbin 的
        // NSDMI 預設是 false，所以今天仍然走 331 那一支。
        if (fBinSel->chkShow0Xbin->Checked)   // jou 20220719: show 0X bin -- golden :2237
            fShowBinSelect->Width = 400;      // kevin 20140317: was 280 -- golden :2238
        else
            fShowBinSelect->Width = 331;      // kevin 20140317: was 280 -- golden :2240
    }
}

//---------------------------------------------------------------------------
//  btReturnClick -- golden :2244-2251
//  GATE (B8): PageControl1Change(this) -- Wave B (no-op this wave, see below).
//---------------------------------------------------------------------------
void TfShowBinSelect::btReturnClick(TObject * /*Sender*/)
{
    fShowBinSelect->Left = 756;
    fShowBinSelect->Top = 412;
    fShowBinSelect->Left = 756;
    fShowBinSelect->Top = 420;
    PageControl1Change(nullptr);
}

//---------------------------------------------------------------------------
//  btnCleanResetClick -- golden :2253-2260
//  GATE (B5) NARROWED 20260826 (FW-UNGATE-W29): same shape as
//  ed_AutoCleanCountClick.
//---------------------------------------------------------------------------
void TfShowBinSelect::btnCleanResetClick(TObject * /*Sender*/)
{
    // AI(W906-FW-UNGATE-W29) 20260826: GATE (B5) 收窄 —— 理由同
    // ed_AutoCleanCountClick（見那支的完整說明）。
    if (fSecurity->Insufficient(97))                                            // golden :2255
    {
        // GATE (B5-殘留): fCleaning->btnResetCleanCountClick(Owner);   // golden :2257
        ed_AutoCleanCount->Text = 0;                                            // golden :2258
    }
}

//---------------------------------------------------------------------------
//  btnClearCountClick -- golden :2262-2277
//  GATE (B6): fSecurity->Insufficient(108) forced false (fail-closed) for
//  every non-Greatek customer code; ShowMyMessageBox_YES_NO forced to "No";
//  fCounterClear->ClearCount(ctIndexCount) still gated 20260826 (FW-UNGATE-W29)
//  -- blocked by the (B6) modal, NOT by fCounterClear; see the in-body note.
//---------------------------------------------------------------------------
void TfShowBinSelect::btnClearCountClick(TObject * /*Sender*/)
{
    if (CUSTOMER_CODE != CC_Greatek)   // Sam 201700915 (Steven): Greatek's clear doesn't need permission
    {
        // AI(W906-FW-SecUnlock) 20260819: GATE (B6) permission half DISSOLVED
        // -- fSecurity real (FW-SecCC); Insufficient(108) is false while SEC1
        // stays closed, so this still returns for every non-Greatek customer
        // today (identical observable). The modal half below stays gated.
        if(fSecurity->Insufficient(108)==false)
            return;
    }

    // GATE (B6): int ret=ShowMyMessageBox_YES_NO(...); if(ret==2) return; --
    // untranslated modal (Automation/AGV_E84.h:58's own comment). Forced to
    // "No"/cancel:
    return;

    // GATE (B1) 在這一站點 **不開** —— AI(W906-FW-UNGATE-W29) 20260826：
    // 舊理由（fCounterClear 無 facade）已死（見本檔上方三個已開閘
    // 站點），但這一行的**真正**阻塞物是 (B6) 的 modal：
    // ShowMyMessageBox_YES_NO 仍未翻譯，被 fail-closed 成 "No" 並在
    // 上方無條件 `return;`。把 golden :2276 的
    // `fCounterClear->ClearCount(ctIndexCount);` 寫回來會是 return 後的
    // 死碼（永遠不會執行），所以保留為註解，**理由換成真的那個**。
    // (B6) 的 modal 一落地，這行要跟著還原。
    // golden :2276: fCounterClear->ClearCount(ctIndexCount);
}

//---------------------------------------------------------------------------
//  PageControl1Change -- GATE (B8): documented no-op, WAVE B (golden
//  :1479-1633). Declared only so btReturnClick's call site compiles/links.
//---------------------------------------------------------------------------
void TfShowBinSelect::PageControl1Change(TObject * /*Sender*/)
{
    // WAVE B: real body (golden :1479-1633) touches ~15 more widgets
    // (pnlShowBin, ScrollBox1, iFixMax-bounded loops, ...) not in this
    // facade yet. See forms/fShowBinSelect.h WAVE B QUEUE.
}

//---------------------------------------------------------------------------
//  ShowBinSel -- golden :388-756 (JerryYang 20220909: BinTrayTotal->eTrayCount)
//  AI(W906-FW-SBWB2) 20260819: FW-3 Wave B, primary target.
//
//  DEVIATION: golden builds two LOCAL arrays right at function entry,
//  `TPanel *UnLoadPanel[]={pnlAuto1,pnlAuto2,...}` / `TLabel *UnLoadLabel[]=
//  {lblAuto1,lblAuto2,...}` (golden :394-404), out of 33 individually-named
//  .dfm widgets. Absence-claim grep this wave (`grep -n "\bpnlAuto1\b"
//  cShowBinSelect.cpp`, `grep -n "\blblAuto1\b" cShowBinSelect.cpp`,
//  20260819): each of the 33 names is referenced in exactly ONE other place
//  in the whole golden class besides this literal-array line -- the still-
//  BLOCKED Wave C `ChangeBinDispStatus`'s OWN local array (golden :210-215,
//  opaque HSys.BinDisCtrl body). No golden method ever names e.g. `pnlAuto1`
//  a THIRD time. Same "array member, not ~33 individually named pointers"
//  shape as this file's own WAVE A ctor DEVIATION note for MyBinSel/
//  MyBinSelLab/etc -- `UnLoadPanel[]`/`UnLoadLabel[]` are therefore
//  TfShowBinSelect MEMBER arrays (forms/fShowBinSelect.h), populated once in
//  the ctor, referenced directly here (no local re-declaration).
//
//  HYDRATION: no per-slot .dfm geometry/Caption/Color hydrated for
//  UnLoadPanel[]/UnLoadLabel[]/pnlEmpty/pnlColor -- see forms/
//  fShowBinSelect.h HYDRATION section for the grep evidence (ShowBinSel
//  itself unconditionally overwrites ->Color for every slot with
//  Prod.iTrayType[i]!=tNotUse before anything in this wave's scope ever
//  reads it back; the tNotUse slots it skips are exactly the slots
//  SetAutoVisible/SetLabelVisible (Wave A, already ACTIVE) hides via
//  grpBinDisp[i]->Visible=false).
//
//  GATE (B9): fSortCT->myCountPanel[i].pnlYield/.pnlCount->Font->Color
//  (golden :640/:642/:648/:650) -- see forms/fShowBinSelect.h GATE REGISTER.
//  GATE (B10)/(B11): ShowBinSel_ARTNor()/ShowBinSel_ARTRT() (golden :752-753)
//  -- each still WAVE B QUEUE, declared below as documented no-ops (same
//  established shape as this file's existing (B8) PageControl1Change).
//  GOLDEN BUG (B12): see forms/fShowBinSelect.h GATE REGISTER -- kept
//  verbatim at its own line below.
//---------------------------------------------------------------------------
void TfShowBinSelect::ShowBinSel()
{
    int Data, iLengh = 0, i2 = 0;
    bool iRecord[100] = {false};
    AnsiString S[eTrayCount], asBuf;

    if (IniConfig.bVTESTFunction == true)   // RogerYang 20250224: Weitest requested 3 more columns
    {
        UPH_StringGrid->ColCount = 7;
    }

    for (int i = 0; i < eTrayCount; i++)   // JerryYang 20220909: BinTrayTotal -> eTrayCount
    {
        S[i] = "";                          // JerryYang 20230512: default blank string
        BinAssign[i] = "";
        bUnloadHasBin[i] = false;           // kevin 20180705: mark whether the tray has a BIN set
        iTrayLastBin[i] = 0;                // kevin 20180705: each tray's last bin
    }

    for (int j = 0; j < eTrayCount; j++)   // JerryYang 20220909: iBinSelCT -> eTrayCount
        iBinTray[j] = false;                // kevin 20170328 (Steven): whether TRAY uses BIN

    for (int i = 0; i < iTestBinCount; i++)
    {
        Data = Prod.iT6CatData[i];
        if (Data < 0)   // kevin 20140317: 256 bin, 0-start
            continue;

        for (int j = 0; j < eTrayCount; j++)
        {
            if (grpBinDisp[j]->Visible == false)
                continue;

            if (j == Data)
            {
                if (fBinSel->chkShow0Xbin->Checked)   // jou 20220719: show 0X bin
                {
                    asBuf.sprintf("%02d ", i);
                    S[j] += asBuf;
                }
                else
                {
                    if (IniConfig.bSPILFunction == true &&
                        CosFunction.bSortingBy2DList == true &&
                        LastSet.iTester == _2D_SORT &&
                        TestIF_File.bSortingBy2DIDList == true)   // JerryYang 20230322: SPIL re-IT requirement
                    {
                        if (S[j] == "")
                        {
                            S[j] = AnsiString(i);   // kevin 20140317: bin 0-start
                        }
                        else
                        {
                            S[j] += " " + AnsiString(i);   // kevin 20140317: bin 0-start
                        }
                    }
                    else
                    {
                        S[j] += AnsiString(i) + " ";   // kevin 20140317: bin 0-start
                    }
                }

                sBinData[j]      = AnsiString(i) + " ";   // kevin 20180202: record bin
                iBinTray[j]      = true;                   // kevin 20170328 (Steven)
                iLengh           = S[j].Length();
                BinAssign[j]    += AnsiString(i) + ",";    // kevin 20180705
                bUnloadHasBin[j] = true;                    // kevin 20180705
                iTrayLastBin[j]  = i;                       // kevin 20180705
                i2               = iLengh / 460;
                if (i2 != 0)
                {
                    if (iRecord[i2] == false)
                    {
                        iRecord[i2]   = true;
                        S[j]         += "\n\r";
                        sBinData[j]   = "\n\r";              // kevin 20180202
                        BinAssign[j] += "\n\r";               // kevin 20180705
                    }
                }
            }
            else
            {
                if (CosFunction.bSortingBy2DList == true &&
                    LastSet.iTester == _2D_SORT &&
                    TestIF_File.bSortingBy2DIDList == true)   // Frank 20221122: 2DID sorting for ATK
                {
                }
                else if (IniConfig.bSPILFunction == true)   // JerryYang 20250320: SPIL shared-tray convention
                {
                }
                else
                {
                    if (iTestBinCount <= 16)   // jou 2014-04-30: 15-bin repeat readability
                    {
                        S[j]         += ". ";
                        BinAssign[j] += ",";     // kevin 20180705
                    }
                }
            }
        }
    }

    Data = Prod.iIfErrorT6;
    if (TestIF_File.iTestType == RS232_MODE &&
        TestIF_File.iRs232Mode == eRs23232Bin)   // Steven 20121112: RS232 supports 32-bin
    {
        if (Data > 0)
        {
            S[Data]             = " E" + S[Data];
            sBinData[Data]      = " E" + S[Data];   // kevin 20180202
            iBinTray[Data]      = true;               // kevin 20170328 (Steven)
            BinAssign[Data]    += "E";                 // kevin 20180705
            bUnloadHasBin[Data] = true;                 // kevin 20180705
            iTrayLastBin[Data]  = 999;                  // kevin 20180705: Error bin
        }
    }
    else
    {
        if (iHWFix_BinBox == 1 || Data == -1)   // kevin 20160819 // Steven 20230929: && -> ||
        {
            S[eBulkBox]            += "E ";
            sBinData[eBulkBox]     += "E ";    // kevin 20180202
            iBinTray[eBulkBox]      = true;     // kevin 20170328 (Steven)
            BinAssign[eBulkBox]    += "E";      // kevin 20180705
            bUnloadHasBin[eBulkBox] = true;     // kevin 20180705
            iTrayLastBin[eBulkBox]  = 999;      // kevin 20180705: Error bin
        }
        else if (Data > 0)
        {
            if (IniConfig.bSPILFunction == true &&
                CosFunction.bSortingBy2DList == true &&
                LastSet.iTester == _2D_SORT &&
                TestIF_File.bSortingBy2DIDList == true)   // JerryYang 20230322: SPIL re-IT requirement
            {
                S[Data] += "ERR";
            }
            else
            {
                S[Data] += "E ";
            }
            sBinData[Data]      += "E ";   // kevin 20180202
            iBinTray[Data]       = true;    // kevin 20170328 (Steven)
            BinAssign[Data]     += "E";     // kevin 20180705
            bUnloadHasBin[Data]  = true;     // kevin 20180705
            iTrayLastBin[Data]   = 999;      // kevin 20180705: Error bin
            // GOLDEN BUG (B12): the very next statement (golden :545, kevin
            // 20220906) immediately overwrites the `=true` two lines above
            // back to `false` -- for this specific "Data>0, non-BulkBox
            // error-bin reroute" arm, bUnloadHasBin[Data] therefore ALWAYS
            // ends up false, making golden :543's `=true` dead code. Kept
            // verbatim (translation-fidelity policy) -- not "fixed" to
            // `=true` since that would change observable behaviour without
            // user sign-off. See forms/fShowBinSelect.h GATE REGISTER (B12).
            bUnloadHasBin[Data] = false;   // kevin 20220906
        }
    }

    if (iTestBinCount <= 16)
    {
        for (int i = 0; i < eTrayCount; i++)   // JerryYang 20220909: BinTrayTotal -> eTrayCount
        {
            S[i] += ". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ";
            if (IniConfig.bSPILFunction == true)   // JerryYang 20250320: SPIL shared-tray convention
            {
                AnsiString strTemp = BinAssign[i];
                int len = strTemp.Length();

                if (len > 0 && strTemp[len] == ',')
                {
                    BinAssign[i] = BinAssign[i].SubString(1, len - 1);
                }
            }
            else
            {
                BinAssign[i] += ",";   // kevin 20180705
            }
        }
    }

    for (int i = eFix2; i <= iFixMax; i++)
    {
        if (Prod.bLinkTo6Tray[i] == true)
        {
            if (S[i - 1].AnsiPos("LINK") == 0)
                S[i] = AnsiString("LINK ") + S[i - 1];
            else
                S[i] = S[i - 1];
            BinAssign[i] = BinAssign[i - 1];
        }
    }

    // Ifor 20231122: add Magazine Link
    if (AUTO3_IS_MAGAZINE == 1)
    {
        for (int i = 1; i < 14; i++)
        {
            if (BinSelect[iTestRunMode].bMagazineLink[i] == true)
            {
                for (int j = 0; j < 14; j++)
                {
                    // GOLDEN NOTE (not separately numbered -- see forms/
                    // fShowBinSelect.h GATE REGISTER (B12) note): if
                    // bMagazineLink[i..0] were ALL true, this inner loop
                    // would index bMagazineLink[i-j] with a negative
                    // subscript once j>i (out-of-bounds read). Unreachable in
                    // practice because bMagazineLink[eAuto1] (index 0) is
                    // never set true by any writer this tree ports, so the
                    // loop always breaks at/before j==i. Kept verbatim.
                    if (BinSelect[iTestRunMode].bMagazineLink[i - j] == false)
                    {
                        S[eMag1 + i] = AnsiString("LINK ") + S[eMag1 + (i - j)];
                        BinAssign[eMag1 + i] = BinAssign[eMag1 + (i - j)];
                        break;
                    }
                }
            }
        }
    }

    for (int i = 0; i < eTrayCount; i++)   // JerryYang 20220909: 10 -> eTrayCount // kevin 20160819/20110901: use FIX for 2 trays
    {
        MyBinSel[i]->Caption = S[i];
        if (S[i].Pos("LINK ") > 0)
        {
            UnLoadLabel[i]->Caption = DelDot(MyBinSel[i]->Caption.SubString(6, MyBinSel[i]->Caption.Length()));
        }
        else
        {
            UnLoadLabel[i]->Caption = DelDot(MyBinSel[i]->Caption);
        }

        // GOLDEN ODDITY (B13, integration 20260819): this gray-out branch is DEAD
        // in golden itself -- the 87-char SPACE-led needle below appears ONLY in
        // Pos() checks (golden :628 and both ART siblings :2543/:2762); the only
        // dots writer is the 80-char DOT-led literal (:561), and no code path
        // ever produces a caption BEGINNING with the needle (measured: needle not
        // in write, not in "0 "+write; j==1 unreachable). The colored else-branch
        // always runs for non-tNotUse trays. Translated verbatim, not simplified.
        int j = MyBinSel[i]->Caption.Pos(" . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ");

        if (iHWFix_BinBox == 1 && i == eBulkBox)   // kevin 20160825
            continue;

        if (Prod.iTrayType[i] == tNotUse)
            continue;

        if (iTestBinCount <= 16 && j == 1)
        {
            UnLoadPanel[i]->Color       = clGray;
            UnLoadLabel[i]->Font->Color = clGray;
            // GATE (B9): fSortCT->myCountPanel[i].pnlYield->Font->Color=clGray;
            MyBinSel[i]->Font->Color    = clGray;
            // GATE (B9): fSortCT->myCountPanel[i].pnlCount->Font->Color=clGray;  // kevin 20150615
        }
        else
        {
            UnLoadPanel[i]->Color       = tcBinColor[Prod.iIsFailT6[i]];
            UnLoadLabel[i]->Font->Color = tcBinColor[Prod.iIsFailT6[i]];
            // GATE (B9): fSortCT->myCountPanel[i].pnlYield->Font->Color=tcBinColor[Prod.iIsFailT6[i]];
            MyBinSel[i]->Font->Color    = tcBinColor[Prod.iIsFailT6[i]];
            // GATE (B9): fSortCT->myCountPanel[i].pnlCount->Font->Color=tcBinColor[Prod.iIsFailT6[i]];  // kevin 20150615
        }

        MyBinSel[i]->Caption = S[i];   // JerryYang 20250320: SPIL shared-tray convention (redundant re-write, same value as above -- kept verbatim)
        if (IniConfig.bSPILFunction == true)
        {
            sSVBinAssign[i] = BinAssign[i];
        }
        else if (fAGV->IsATK_AMR())
        {
            int iLen = BinAssign[i].Length();   // RogerYang 20260403: remove trailing comma
            AnsiString sGetCommas = BinAssign[i].SubString(iLen, iLen);
            if (sGetCommas == ",")
                sSVBinAssign[i] = BinAssign[i].SubString(0, iLen - 1);
            else
                sSVBinAssign[i] = BinAssign[i];

            sSVBinAssign[i] = StringReplace(sSVBinAssign[i], "\r", "", TReplaceFlags() << rfReplaceAll);   // AI(ht9045-atk-amr-flow) 20260423 (RogerYang): strip CR/LF
            sSVBinAssign[i] = StringReplace(sSVBinAssign[i], "\n", "", TReplaceFlags() << rfReplaceAll);

            if (sBinCode_ATK[i] != BinAssign[i])
            {
                sBinCode_ATK[i] = BinAssign[i];
                EventReport(SECS_EVENT.OutputPort1BinCode + i);
            }
        }
        else
        {
            sSVBinAssign[i] = MyBinSel[i]->Caption;
        }
    }

    if (fAGV->IsATK_AMR())   // AI(ht9045-atk-amr-flow) 20260423 (RogerYang): sync BinAssign to Fix(Auto4~6)
    {
        for (int k = 0; k < 3; k++)
            sSVBinAssign[eAuto4 + k] = sSVBinAssign[eFix1 + k];
    }

    if (AUTO3_IS_MAGAZINE != 1)
    {
        for (int i = eMag1; i < eMag1 + 14; i++)
        {
            UnLoadLabel[i]->Caption = ". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ";
            UnLoadPanel[i]->Caption = "X";
            if (NUMBER_PANEL_TYPE == 3)   // 3-digit display
            {
                UnLoadPanel[i]->Color       = (TColor)0x000080FF;
                UnLoadLabel[i]->Font->Color = (TColor)0x000080FF;
            }
            else
            {
                UnLoadPanel[i]->Color       = clGray;
                UnLoadLabel[i]->Font->Color = clGray;
            }
        }
    }

    if (TrayForm.iFixTrayMode == false)   // remaining 3 Fix trays only when not split in half
    {
        for (int i = iFixRight + 1; i <= iFixRightHalf; i++)
        {
            UnLoadLabel[i]->Caption = ". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ";
            UnLoadPanel[i]->Caption = "X";
            if (NUMBER_PANEL_TYPE == 3 ||   // 3-digit display
                NUMBER_PANEL_TYPE == 4)      // Sam 20240604: new BinDisplay TFT
            {
                UnLoadPanel[i]->Color       = (TColor)0x000080FF;
                UnLoadLabel[i]->Font->Color = (TColor)0x000080FF;
            }
            else
            {
                UnLoadPanel[i]->Color       = clGray;
                UnLoadLabel[i]->Font->Color = clGray;
            }
        }
    }

    if (AUTO_EMPTY_COLOR != 0)   // has Empty/Color trays
    {
        pnlEmpty->Color = (TColor)0x000080FF;
        pnlColor->Color = (TColor)0x000080FF;
    }
    else
    {
        pnlEmpty->Color = clGray;
        pnlColor->Color = clGray;
    }

    if (IniConfig.bAutoTrayLink == true)   // jou 2012-06-14: Auto Tray Link
    {
        for (int i = eAuto2; i <= iAutoRight; i++)
        {
            if (Prod.bLinkTo6Tray[i] == true)
            {
                if (MyBinSel[i - 1]->Caption.AnsiPos("LINK") == 0)
                    MyBinSel[i]->Caption = AnsiString("LINK ") + MyBinSel[i - 1]->Caption;
                else
                    MyBinSel[i]->Caption = MyBinSel[i - 1]->Caption;
            }
        }
    }

    // GATE (B10)/(B11): ShowBinSel_ARTNor()/ShowBinSel_ARTRT() -- each still
    // WAVE B QUEUE (golden :2365-2580 / :2581-2799), declared as documented
    // no-ops below (see forms/fShowBinSelect.h GATE REGISTER).
    ShowBinSel_ARTNor();   // JerryYang 20220331: also show ART bin
    ShowBinSel_ARTRT();

    // S: golden passes `this` (TfShowBinSelect*->TObject* in VCL);
    // TfShowBinSelect has no vclcompat::TObject base (same shape as every
    // other facade in this family -- see forms/fContactCT.h's own note) and
    // PageControl1Change ignores Sender entirely (GATE (B8), documented
    // no-op) -- nullptr is semantics-identical. Same substitution shape as
    // this file's own btReturnClick, which already calls
    // `PageControl1Change(nullptr)` for the identical reason.
    PageControl1Change(nullptr);
}

//---------------------------------------------------------------------------
//  ShowBinSel_ARTNor -- golden :2365-2580. GATE (B10) DISSOLVED by
//  AI(W906-FW-SBWC) 20260819 -- real body lands this wave (tsARTNormalBin/
//  palARTNor/MyBinSelARTFT[] now real, see forms/fShowBinSelect.h WAVE C
//  SCOPE). 0 residual GATEs.
//
//  NOTES (translation-fidelity, not GATEs):
//  * GOLDEN ODDITY (not separately numbered -- informational, not a GATE):
//    `iBinTray[9]`/`S[9]`/`sBinData[9]`/`BinAssign[9]`/`bUnloadHasBin[9]`/
//    `iTrayLastBin[9]` below use the LITERAL `9`, exactly as golden writes
//    it here -- but `9` is NOT the same slot as the sibling, already-ported
//    ShowBinSel's `eBulkBox` (golden :531-538). Checked this wave
//    (MachineType.h:1089): `eBulkBox==18`, not `9` -- `9` is one past this
//    function's OWN `BinTrayTotal` (9 default / 10 when `iHWFix_BinBox==1`,
//    i.e. 6 Auto + Fix1-3), which in the modern `eTrayCount`-wide numbering
//    this function's `S[eTrayCount]`/`MyBinSelARTFT[e3TrayCount]` arrays
//    actually use lands on `eFix4`'s slot (index 9), NOT the Magazine-era
//    `eBulkBox` slot (index 18) ShowBinSel writes to for the identical
//    "iHWFix_BinBox error-bin fallback" concept. Kept verbatim either way
//    (translation-fidelity policy) -- this is a genuine behavioural
//    divergence between the two sibling functions in golden itself, not a
//    porting choice.
//  * Unlike ShowBinSel (golden :622, `if(Prod.iTrayType[i]==tNotUse)
//    continue;`), this function's own colouring loop has NO such check --
//    every slot 0..eTrayCount-1 gets a real Caption/Font->Color write
//    regardless of whether that tray is actually in use. Kept verbatim.
//  * The `Pos(" . . . ")` gray/colour check a few lines below is the SAME
//    dead branch as ShowBinSel's own (GATE (B13), forms/fShowBinSelect.h
//    GATE REGISTER / cShowBinSelect.cpp's ShowBinSel banner) -- no writer
//    ever produces a caption beginning with the 87-char space-led needle,
//    so the `clGray` arm is unreachable and the coloured `else` always runs.
//---------------------------------------------------------------------------
void TfShowBinSelect::ShowBinSel_ARTNor()                            //JerryYang 20220331 : 緯平要求show ART BIN
{
    if(IniConfig.bSPILFunction && bCanRunSCKART)
    {
        tsARTNormalBin->TabVisible=true;
        int Data, BinTrayTotal=9;
        AnsiString S[eTrayCount], Str;
        if(iHWFix_BinBox==1)                                                    //kevin 20160819
        {
            BinTrayTotal=10;
        }

        if(iTestRunMode==FT)
        {
            palARTNor->Color=clGreen;
        }
        else
        {
            palARTNor->Color=clBtnFace;
        }

        for(int i=eAuto1; i<=iAutoRight; i++)
        {
            if(Prod.bTo6AutoRetest[FT][i]==true)
                Str.sprintf("%s (R)", s6TrayName[i]);
            else
                Str.sprintf("%s", s6TrayName[i]);
            MyBinSelARTFT[i]->Caption=Str;
        }

        for(int i=0; i<BinTrayTotal; i++)
        {
            S[i]=" ";
            BinAssign[i]="";
            bUnloadHasBin[i]=false;                                             //kevin 20180705 有BIN TRAY set true
            iTrayLastBin[i]=0;                                                  //kevin 20180705 每一個TRAY最後一個BIN
        }

        int iLengh=0, i2=0;
        bool iRecord[100]={false};
        for(int i=0; i<eTrayCount; i++)
            iBinTray[i]=false;                                                  //kevin 20170328 (Steven) add 判斷TRAY 是否使用 BIN

        for(int i=0; i<iTestBinCount; i++)
        {
            Data=Prod.iTo6CatData[FT][i];
            if(Data<0)                                                         //JerryYang 20251020 : fix ART顯示錯誤  //kevin 20140317 256 bin 0 start
                continue;

            for(int j=0; j<eTrayCount; j++)                                     //JerryYang 20251020 : fix ART顯示錯誤
            {
                if(j==Data)
                {
                    S[j]+=AnsiString(i)+" ";                                    //kevin 20140317 bin 0 start
                    sBinData[j]=AnsiString(i)+" ";                              //kevin 20180202 record bin
                    iBinTray[j]=true;                                           //kevin 20170328 (Steven) add 判斷TRAY 是否使用 BIN
                    iLengh=S[j].Length();
                    i2=iLengh/460;

                    BinAssign[j]+=AnsiString(i)+",";                            //kevin 20180705 add Auto 123 fix 123 bin data
                    bUnloadHasBin[j]=true;                                      //kevin 20180705 有BIN TRAY set true
                    iTrayLastBin[j]=i;                                          //kevin 20180705 每一個TRAY最後一個BIN

                    if(i2!=0)
                    {
                        if(iRecord[i2]==false)
                        {
                            iRecord[i2]=true;
                            S[j]+="\n\r";
                            sBinData[j]="\n\r";                                 //kevin 20180202 record bin
                            BinAssign[j]+="\n\r";                               //kevin 20180705 add Auto 123 fix 123 bin data
                        }
                    }
                }
                else
                {
                    if(iTestBinCount<=16)                                       //jou 2014-04-30 15 Bin 回復 ... 比較好閱讀
                    {
                        S[j]+=". ";
                        BinAssign[j]+=",";                                      //kevin 20180705 add Auto 123 fix 123 bin data
                    }
                }
            }
        }

        Data=Prod.iTo6IfError[FT];

        if(TestIF_File.iTestType==RS232_MODE && TestIF_File.iRs232Mode==eRs23232Bin)    //Steven 20121112 : RS232支援32Bin
        {
            if(Data>=0)
            {
                S[Data]=" E"+S[Data];
                sBinData[Data]=" E"+S[Data];                                    //kevin 20180202 record bin
                iBinTray[9]=true;                                               //kevin 20170328 (Steven) add 判斷Fix TRAY 是否使用 BIN
                BinAssign[Data]+="E";                                           //kevin 20180705 add Auto 123 fix 123 bin data
                bUnloadHasBin[Data]=true;                                       //kevin 20180705 有BIN TRAY set true
                iTrayLastBin[Data]=999;                                         //kevin 20180705 每一個TRAY最後一個BIN  Error bin
            }
        }
        else
        {
            if(iHWFix_BinBox==1 && Data==-1)                                    //kevin 20160819
            {
                S[9]+="E ";
                sBinData[9]+="E ";                                              //kevin 20180202 record bin
                iBinTray[9]=true;                                               //kevin 20170328 (Steven) add 判斷Fix TRAY 是否使用 BIN
                BinAssign[9]+="E";                                              //kevin 20180705 add Auto 123 fix 123 bin data
                bUnloadHasBin[9]=true;                                          //kevin 20180705 有BIN TRAY set true
                iTrayLastBin[9]=999;                                            //kevin 20180705 每一個TRAY最後一個BIN  Error bin
            }
            else if(Data>=0)
            {
                S[Data]+="E ";
                sBinData[Data]+="E ";                                           //kevin 20180202 record bin
                iBinTray[9]=true;                                               //kevin 20170328 (Steven) add 判斷Fix TRAY 是否使用 BIN
                BinAssign[Data]+="E";                                           //kevin 20180705 add Auto 123 fix 123 bin data
                bUnloadHasBin[Data]=true;                                       //kevin 20180705 有BIN TRAY set true
                iTrayLastBin[Data]=999;                                         //kevin 20180705 每一個TRAY最後一個BIN  Error bin
            }
        }

        //jou 2014-04-30 15 Bin 回復 ... 比較好閱讀
        if(iTestBinCount<=16)
        {
            for(int i=0; i<BinTrayTotal; i++)                                   //kevin 20140317 bin
            {
                S[i]+=". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ";
                BinAssign[i]+=",";                                              //kevin 20180705 add Auto 123 fix 123 bin data
            }
        }

        //Steven 20181113 : 修正Fix Link顯示問題
        //==>
        if(Prod.bTo6AutoLink[FT][eFix2]==true)
        {
            S[4]=AnsiString("LINK ")+S[3];
            BinAssign[4]=BinAssign[3];
        }

        if(Prod.bTo6AutoLink[FT][eFix2]==true &&
           Prod.bTo6AutoLink[FT][eFix3]==true)
        {
            S[5]=AnsiString("LINK ")+S[3];
            BinAssign[5]=BinAssign[3];
        }
        else if(Prod.bTo6AutoLink[FT][eFix2]==false &&
                Prod.bTo6AutoLink[FT][eFix3]==true)
        {
            S[5]=AnsiString("LINK ")+S[4];
            BinAssign[5]=BinAssign[4];
        }
        //<==
        //Steven 20181113 : 修正Fix Link顯示問題
        //Ifor 20231122 add Magazine Link
        //==>
        if(AUTO3_IS_MAGAZINE==1)
        {
            for(int i=1; i<14; i++)
            {
                if(BinSelect[FT].bMagazineLink[i]==true)
                {
                    for(int j=0; j<14; j++)
                    {
                        if(BinSelect[FT].bMagazineLink[i-j]==false)
                        {
                            S[eMag1+i]=AnsiString("LINK ")+S[eMag1+(i-j)];
                            BinAssign[eMag1+i]=BinAssign[eMag1+(i-j)];
                            break;
                        }
                    }
                }
            }
        }
        //<==
        //Ifor 20231122 add Magazine Link
        for(int i=0; i<eTrayCount; i++)                                         //JerryYang 20251020 : fix ART顯示錯誤
        {
            MyBinSelARTFT[i]->Caption=S[i];
            int j=MyBinSelARTFT[i]->Caption.Pos(" . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ");    //kevin 20140317 bin
            if(iHWFix_BinBox==1 && i==eBulkBox)                                 //kevin 20160825
                continue;
            if(iTestBinCount<=16 && j==1)
            {
                MyBinSelARTFT[i]->Font->Color                    =clGray;
            }
            else
            {
                MyBinSelARTFT[i]->Font->Color                    =tcBinColor[Prod.iTo6StackDefFailCate[FT][i]];
            }
        }

        if(IniConfig.bAutoTrayLink==true)                                       //jou 2012-06-14 Auto Tray Link
        {
            if(Prod.bTo6AutoLink[FT][eFix2]==true)
            {
                MyBinSelARTFT[1]->Caption=MyBinSelARTFT[0]->Caption;
            }

            if(Prod.bTo6AutoLink[FT][eFix2]==true &&
               Prod.bTo6AutoLink[FT][eFix3]==true)
            {
                MyBinSelARTFT[2]->Caption=MyBinSelARTFT[0]->Caption;
            }
            else if(Prod.bTo6AutoLink[FT][eFix2]==false &&
                    Prod.bTo6AutoLink[FT][eFix3]==true)
            {
                MyBinSelARTFT[2]->Caption=MyBinSelARTFT[1]->Caption;
            }
        }
    }
    else
    {
        tsARTNormalBin->TabVisible=false;
    }
}

//---------------------------------------------------------------------------
//  ShowBinSel_ARTRT -- golden :2581-2799. GATE (B11) DISSOLVED by
//  AI(W906-FW-SBWC) 20260819 -- real body lands this wave (tsARTRTBin/
//  palARTRT/MyBinSelARTRT[] now real). Same shape/notes as
//  ShowBinSel_ARTNor above (literal `9` for the error-bin slot; no
//  `tNotUse` skip in the colouring loop; shares GATE (B13)'s dead branch).
//---------------------------------------------------------------------------
void TfShowBinSelect::ShowBinSel_ARTRT()                             //JerryYang 20220331 : 緯平要求show ART BIN
{
    if(IniConfig.bSPILFunction && bCanRunSCKART)
    {
        tsARTRTBin->TabVisible=true;
        int Data, iBinSelCT=0, BinTrayTotal=9;
        AnsiString S[eTrayCount];

        if(iHWFix_BinBox==1)                                                    //kevin 20160819
        {
            BinTrayTotal=10;
        }

        for(int i=0; i<BinTrayTotal; i++)
        {
            S[i]=" ";
            BinAssign[i]="";
            bUnloadHasBin[i]=false;                             //kevin 20180705 有BIN TRAY set true
            iTrayLastBin[i]=0;                                  //kevin 20180705 每一個TRAY最後一個BIN
        }

        if(iTestRunMode==RT)
        {
            palARTRT->Color=clGreen;
        }
        else
        {
            palARTRT->Color=clBtnFace;
        }

        if(TrayForm.iFixTrayMode)
        {
            iBinSelCT=9;                                                        //use up down
        }
        else
        {
            iBinSelCT=6;
        }
        int iLengh=0,i2=0;
        bool iRecord[100]={false};
        for(int j=0; j<iBinSelCT; j++)
            iBinTray[j]=false;                                                  //kevin 20170328 (Steven) add 判斷TRAY 是否使用 BIN

        for(int i=0; i<iTestBinCount; i++)
        {
            Data=Prod.iTo6CatData[RT][i];
            if(Data<0)                                                          //JerryYang 20251020 : fix ART顯示錯誤  //kevin 20140317 256 bin 0 start
              continue;

            for(int j=0; j<eTrayCount; j++)                                     //JerryYang 20251020 : fix ART顯示錯誤
            {
                if(j==Data)
                {
                    S[j]+=AnsiString(i)+" ";                                    //kevin 20140317 bin 0 start
                    sBinData[j]=AnsiString(i)+" ";                              //kevin 20180202 record bin
                    iBinTray[j]=true;                                           //kevin 20170328 (Steven) add 判斷TRAY 是否使用 BIN
                    iLengh=S[j].Length();
                    i2=iLengh/460;

                    BinAssign[j]+=AnsiString(i)+",";                            //kevin 20180705 add Auto 123 fix 123 bin data
                    bUnloadHasBin[j]=true;                                      //kevin 20180705 有BIN TRAY set true
                    iTrayLastBin[j]=i;                                          //kevin 20180705 每一個TRAY最後一個BIN

                    if(i2 !=0)
                    {
                        if(iRecord[i2]==false)
                        {
                            iRecord[i2]=true;
                            S[j]+="\n\r";
                            sBinData[j]="\n\r";                                 //kevin 20180202 record bin
                            BinAssign[j]+="\n\r";                               //kevin 20180705 add Auto 123 fix 123 bin data
                        }
                    }
                }
                else
                {
                    //jou 2014-04-30 15 Bin 回復 ... 比較好閱讀
                    if(iTestBinCount<=16)
                    {
                        S[j]+=". ";
                        BinAssign[j]+=",";                                      //kevin 20180705 add Auto 123 fix 123 bin data
                    }
                }
            }
        }

        Data=Prod.iTo6IfError[RT];

        if(TestIF_File.iTestType==RS232_MODE &&
           TestIF_File.iRs232Mode==eRs23232Bin)                                 //Steven 20121112 : RS232支援32Bin
        {
            if(Data>=0)
            {
                S[Data]=" E"+S[Data];
                sBinData[Data]=" E"+S[Data];                    //kevin 20180202 record bin
                iBinTray[9]=true;                               //kevin 20170328 (Steven) add 判斷Fix TRAY 是否使用 BIN
                BinAssign[Data]+="E";                           //kevin 20180705 add Auto 123 fix 123 bin data
                bUnloadHasBin[Data]=true;                       //kevin 20180705 有BIN TRAY set true
                iTrayLastBin[Data]=999;                         //kevin 20180705 每一個TRAY最後一個BIN  Error bin
            }
        }
        else
        {
            if(iHWFix_BinBox==1 && Data==-1)                    //kevin 20160819
            {
                S[9]+="E ";
                sBinData[9]+="E ";                              //kevin 20180202 record bin
                iBinTray[9]=true;                               //kevin 20170328 (Steven) add 判斷Fix TRAY 是否使用 BIN
                BinAssign[9]+="E";                              //kevin 20180705 add Auto 123 fix 123 bin data
                bUnloadHasBin[9]=true;                          //kevin 20180705 有BIN TRAY set true
                iTrayLastBin[9]=999;                            //kevin 20180705 每一個TRAY最後一個BIN  Error bin
            }
            else if(Data>=0)
            {
                S[Data]+="E ";
                sBinData[Data]+="E ";                           //kevin 20180202 record bin
                iBinTray[9]=true;                               //kevin 20170328 (Steven) add 判斷Fix TRAY 是否使用 BIN
                BinAssign[Data]+="E";                           //kevin 20180705 add Auto 123 fix 123 bin data
                bUnloadHasBin[Data]=true;                       //kevin 20180705 有BIN TRAY set true
                iTrayLastBin[Data]=999;                         //kevin 20180705 每一個TRAY最後一個BIN  Error bin
            }
        }

        //jou 2014-04-30 15 Bin 回復 ... 比較好閱讀
        if(iTestBinCount<=16)
        {
            for(int i=0; i<BinTrayTotal; i++)                                   //kevin 20140317 bin
            {
                S[i]+=". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ";
                BinAssign[i]+=",";                                              //kevin 20180705 add Auto 123 fix 123 bin data
            }
        }

        //Steven 20181113 : 修正Fix Link顯示問題
        //==>
        if(Prod.bTo6AutoLink[RT][eFix2]==true)
        {
            S[4]=AnsiString("LINK ")+S[3];
            BinAssign[4]=BinAssign[3];
        }

        if(Prod.bTo6AutoLink[RT][eFix2]==true &&
           Prod.bTo6AutoLink[RT][eFix3]==true)
        {
            S[5]=AnsiString("LINK ")+S[3];
            BinAssign[5]=BinAssign[3];
        }
        else if(Prod.bTo6AutoLink[RT][eFix2]==false &&
                Prod.bTo6AutoLink[RT][eFix3]==true)
        {
            S[5]=AnsiString("LINK ")+S[4];
            BinAssign[5]=BinAssign[4];
        }
        //<==
        //Steven 20181113 : 修正Fix Link顯示問題
        //Ifor 20231122 add Magazine Link
        //==>
        if(AUTO3_IS_MAGAZINE==1)
        {
            for(int i=1; i<14; i++)
            {
                if(BinSelect[RT].bMagazineLink[i]==true)
                {
                    for(int j=0; j<14; j++)
                    {
                        if(BinSelect[RT].bMagazineLink[i-j]==false)
                        {
                            S[eMag1+i]=AnsiString("LINK ")+S[eMag1+(i-j)];
                            BinAssign[eMag1+i]=BinAssign[eMag1+(i-j)];
                            break;
                        }
                    }
                }
            }
        }
        //<==
        //Ifor 20231122 add Magazine Link

        for(int i=0; i<eTrayCount; i++)                                         //JerryYang 20251020 : fix ART顯示錯誤
        {
            MyBinSelARTRT[i]->Caption=S[i];
            int j=MyBinSelARTRT[i]->Caption.Pos(" . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ");    //kevin 20140317 bin
            if(iHWFix_BinBox==1 && i==eBulkBox)                                 //kevin 20160825
                continue;
            if(iTestBinCount<=16 && j==1)
            {
                MyBinSelARTRT[i]->Font->Color                    =clGray;
            }
            else
            {
                MyBinSelARTRT[i]->Font->Color                    =tcBinColor[Prod.iTo6StackDefFailCate[RT][i]];
            }
        }

        if(IniConfig.bAutoTrayLink==true)                                       //jou 2012-06-14 Auto Tray Link
        {
            if(Prod.bTo6AutoLink[RT][eFix2]==true)
            {
                MyBinSelARTRT[1]->Caption=MyBinSelARTRT[0]->Caption;
            }

            if(Prod.bTo6AutoLink[RT][eFix2]==true &&
               Prod.bTo6AutoLink[RT][eFix3]==true)
            {
                MyBinSelARTRT[2]->Caption=MyBinSelARTRT[0]->Caption;
            }
            else if(Prod.bTo6AutoLink[RT][eFix2]==false &&
                    Prod.bTo6AutoLink[RT][eFix3]==true)
            {
                MyBinSelARTRT[2]->Caption=MyBinSelARTRT[1]->Caption;
            }
        }
    }
    else
    {
        tsARTRTBin->TabVisible=false;
    }
}

//---------------------------------------------------------------------------
//  TimerAutoCleanCountTimer -- golden :2168-2214. AI(W906-FW-SBWC) 20260819:
//  FW-3 Wave C. FULL, 0 GATEs (10 new widgets: rg_FixBinBox,
//  ed_FixBinBoxAlarmCount, LabErrorBinNowCount, PageControl1_ART,
//  Tab_ARTSkipICCount, AutocleanlifeTime, btnCleanReset, BulkBox,
//  labScheduleNAME, labInQty -- see forms/fShowBinSelect.h WAVE C SCOPE).
//  The golden :2207-2213 `CUSTOMER_CODE==CC_JCET` block is commented out IN
//  GOLDEN ITSELF (dead code, not a translation gap) -- kept as a comment
//  below, exactly as golden has it.
//---------------------------------------------------------------------------
void TfShowBinSelect::TimerAutoCleanCountTimer(TObject * /*Sender*/)
{
    if(InitialOK==false)
        return;

    if(IniConfig.bG10ShowImmediateUPH && SystemStart)
        CaculateUPH();                                                          //Steven 20160727 : Show immediate UPH

    if(IniConfig.bEnableAutoCleanFunction==true)                                //Alick 20160624 修改成判斷INI是否啟用AUTO CLEAN功能
    {
        btnCleanReset->Enabled     = true;                                      //Steven 20140514
        ed_AutoCleanCount->Enabled = false;

        if(CUSTOMER_CODE==CC_KYEC_LEE)                                          //Ifor 20160902 add 京元要求Autoclean life time
        {
            AutocleanlifeTime->Caption="Clean Device LifeTime: "+ed_AutoCleanCount->Text+"/"+ IntToStr(TestIF_File.iAutoClean_AlarmCount);
        }
        else
        {
            AutocleanlifeTime->Visible=false;
        }
    }

    BulkBox->Visible=IniConfig.bI33ErrorBinBox;                                 //kevin 20160824 add error bin box
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        labScheduleNAME->Caption=ASET_ScheduleNAME;
        labInQty->Caption=ASET_INTQTY;
    }

    if(CosFunction.bHWBinBox && iHWFix_BinBox==1)                               //kevin 20160822 error bin box 清除
    {
        rg_FixBinBox->ItemIndex=iHWFix_BinBox;                                  //kevin 20160819 error bin 要放到 Bin Box "\\Binasgn.Data";
        ed_FixBinBoxAlarmCount->Text=iErrorBinBoxAlarm;                         //kevin 20160819
        LabErrorBinNowCount->Caption=LastSet.iBinBoxCount;
    }
    PageControl1_ART->TabVisible  =((USE_AUTO_RETEST==eartInstall && IniConfig.bA10_AutoReTest) || CosFunction.bUseARTSortCount);   //kevin 201506016  //wei 20150331 打開功能就顯示
    Tab_ARTSkipICCount->TabVisible=((USE_AUTO_RETEST==eartInstall && IniConfig.bA10_AutoReTest && bForKyecBu3RunART==true) || CosFunction.bUseARTSortCount);  //Frank 20160819    //wei 20161118 bRunART-->bForKyecBu3RunART

//    if(CUSTOMER_CODE==CC_JCET)
//    {                                                                           //jou 20180130 (Steven) : JCET 如春要求機台在生產時SLT lot no不能修改
//        if(HasICUnderMachine())
//            edSLT04->Enabled=false;
//        else
//            edSLT04->Enabled=true;
//    }
}

//---------------------------------------------------------------------------
//  FormShow -- golden :758-866. AI(W906-FW-SBWC) 20260819: FW-3 Wave C.
//  ACTIVE, 1 GATE (B18) -- see forms/fShowBinSelect.h GATE REGISTER.
//
//  SUBSTITUTIONS (S, behaviour-preserving):
//  * `fShowBinSelect->tsUnloadMap`/`fShowBinSelect->palUnloader` (golden
//    :764/:778) kept AS-WRITTEN (the global-pointer-from-within-its-own-
//    method idiom) -- same posture as this file's own btReturnClick
//    (`fShowBinSelect->Left=756;` etc), not `this->`-substituted, for
//    consistency with that established precedent.
//  * `labAuto1`/`labAuto2`/`labAuto3`/`labFix1`/`labFix2`/`labFix3` (golden
//    :779-784, CC_Greatek branch) -> `MyBinSelLab[eAuto1]`/[eAuto2]/[eAuto3]/
//    [eFix1]/[eFix2]/[eFix3]. NOT new facade members -- golden's OWN ctor
//    (golden :56-63, `tempMyBinSelLab[]={labAuto1,labAuto2,labAuto3,...,
//    labFix1,labFix2,labFix3,...}`) assigns these exact pointers into
//    `MyBinSelLab[i]` (golden :124, `MyBinSelLab[i]=tempMyBinSelLab[i];`),
//    so `MyBinSelLab[eAuto1]` IS `labAuto1` in golden -- same object
//    identity, zero new members (see CTOR NOTE / WAVE C SCOPE, forms/
//    fShowBinSelect.h).
//  * `PageControl1Change(this)` (golden :801) -> `PageControl1Change(nullptr)`
//    -- same substitution/reasoning as this file's own btReturnClick/
//    ShowBinSel tail (PageControl1Change ignores Sender entirely, GATE (B8),
//    documented no-op).
//
//  GATE (B18): the CC_GIGAS/SPIL_FOR_QLE inner Top-stacking calculation
//  (`palAutoDeviceEjection->Top=btnASM->Top+btnASM->Height+10;` / the
//  `gbAutoCleanCount` else-arm) -- see forms/fShowBinSelect.h GATE REGISTER.
//  The outer Visible toggle and the `if(palAutoDeviceEjection && ...)` guard
//  are REAL/ACTIVE.
//---------------------------------------------------------------------------
void TfShowBinSelect::FormShow(TObject * /*Sender*/)
{
    if(NUMBER_PANEL_TYPE==3 ||
       NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
        bUpdateBinDigital=true;
    else
        fShowBinSelect->tsUnloadMap->TabVisible=false;                          //Steven 20110411

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20150904
        ed_AutoCleanCount->Visible=false;
    else
        ed_AutoCleanCount->Visible=true;

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20151111 不顯示clean
        btnClearCount->Visible=false;
    else
        btnClearCount->Visible=true;

    if(CUSTOMER_CODE==CC_Greatek)                                               //JimmyChiu 20220124 : 超豐要求改變顯示名稱
    {
        fShowBinSelect->palUnloader->Width=110;
        MyBinSelLab[eAuto1]->Caption="Auto1 (CAT A)";
        MyBinSelLab[eAuto2]->Caption="Auto2 (CAT B)";
        MyBinSelLab[eAuto3]->Caption="Auto3 (CAT C)";
        MyBinSelLab[eFix1]->Caption ="Fix1 (CAT D)";
        MyBinSelLab[eFix2]->Caption ="Fix2 (CAT E)";
        MyBinSelLab[eFix3]->Caption ="Fix3 (JAM)";
    }

    tsFxiAI->TabVisible=USE_Fix_AI_CCD;

    if(CUSTOMER_CODE==CC_ASE_CL)
    {
        tsSECS_Category->TabVisible=true;
    }
    else
    {
        tsSECS_Category->TabVisible=false;
    }

    ShowBinSel();
    bShow=true;
    PageControl1->ActivePage=tsTestBin;
    PageControl1Change(nullptr);   // S: golden passes `this`, see banner note above
    Tab_UPH->TabVisible         =IniConfig.bShowUPH;                            //Steven 20100827
    btnAutoClean->Visible       =IniConfig.bEnableAutoCleanFunction;
    tsASE->TabVisible           =IniConfig.bASE_Report;                         //kevin 20141106 高雄日月光 秀Schedule NUM
    PageControl1_ART->TabVisible=(USE_AUTO_RETEST==eartInstall &&               //kevin 201506016
                                  IniConfig.bA10_AutoReTest);                   //wei 20150331 打開功能就顯示

    if(CosFunction.bUseARTSortCount==true)                                      //Ifor 20170321 add MRT Mode
    {
        PageControl1_ART->TabVisible=true;
        PageControl1_ART->Caption="Category Info AUTO";
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20180313 :add 京元喬智要求 Clean Reset 隱藏 需求 Auto Clean 設定頁面清除
    {
        btnCleanReset->Visible=false;
    }
    else
    {
        btnCleanReset->Visible=IniConfig.bEnableAutoCleanFunction;
    }

    if(CUSTOMER_CODE==CC_UNISEM_M)
    {
        labJamrate->Visible =true;
        Jamrate->Visible    =true;
    }
    Panel1->Visible=CosFunction.IntervalYieldCount;                             //wei 20180606 Interval Low Yield By Site

    pnlSpeciallYield->Visible  =CosFunction.bSpecailLowYeild;                   //Sam 20210505 : PTI 要求的兩段 Low Yeild

    if(IniConfig.bG15LoadInputCount)                                            //kevin 20211106 輸入顆數達成就 Clean out    //JimmyChiu 20220124 fix name
    {
        PLoadInput->Visible=true;
        PLoadInput->Top =120;
        PLoadInput->Left=14;
    }
    else
    {
        PLoadInput->Visible=false;
    }

    tsCategoryInfoContCT->TabVisible=CosFunction.bCategoryInfoByContactCT;       //Sam 20240131 : 新增一組 CategoryInfo 紀錄資料使用 ContactCT
    gbTriggerAlm->Visible=IniConfig.bSPILFunction;                              //RogerYang 20260105 Jimmy Add.
    SetAutoVisible();
    if(CUSTOMER_CODE==CC_GIGAS || SPIL_FOR_QLE==1)                              //JerryYang 20251020 : 渠梁半清機功能
    {
        if(palAutoDeviceEjection)
            palAutoDeviceEjection->Visible=true;

        if(palAutoDeviceEjection &&
           Panel1->Visible==false &&
           pnlSpeciallYield->Visible==false)
        {
            // GATE (B18): if(SPIL_FOR_QLE==1)
            //     palAutoDeviceEjection->Top=btnASM->Top+btnASM->Height+10;   //KevinCheng 20260415 : 避免遮住按鈕
            // else
            //     palAutoDeviceEjection->Top=gbAutoCleanCount->Top+gbAutoCleanCount->Height+10;
            // -- btnASM/gbAutoCleanCount are not on this facade and neither
            // TSpeedButton nor TGroupBox carries ->Top/->Height (pure
            // cosmetic pixel-stacking, never read back); see forms/
            // fShowBinSelect.h GATE REGISTER (B18).
        }
    }
    else
    {
        if(palAutoDeviceEjection)
            palAutoDeviceEjection->Visible=false;
    }
}

// =============================================================================
//  WAVE D (AI(W906-FW3-SBS-WD) 20260820) -- ChangeBinDispStatus /
//  DoShowBinDigital, previously WAVE B QUEUE (BLOCKED by opaque BinDisCtrl).
//  Full rationale, precondition analysis and the (D1)-(D11) GATE REGISTER
//  live in forms/fShowBinSelect.h (WAVE D SCOPE) -- not duplicated here,
//  same "avoid the two files drifting apart" policy as this file's own top
//  banner. Golden ref: cShowBinSelect.cpp golden :207-386 / :997-1423,
//  cp950-decoded this wave (`python3 -c "open(path,'rb').read().decode(
//  'cp950')"`, 0 U+FFFD over the decoded spans, re-verified 20260820).
// =============================================================================
//---------------------------------------------------------------------------
TQPF_Timer AlarmDelay;   // golden :207 -- file-scope global (NOT a class member), matches golden exactly
void TfShowBinSelect::ChangeBinDispStatus()                                     //Steven 20110411 : 顯示目前顯示器的狀態
{
    TPanel *UnLoadPanel[]={pnlLoader, pnlEmpty, pnlColor,
                           pnlAuto1,  pnlAuto2, pnlAuto3,
                           pnlFix1,   pnlFix2,  pnlFix3,  pnlFix4,  pnlFix5,  pnlFix6,  pnlBinBox,
                           pnlMag1,   pnlMag2,  pnlMag3,  pnlMag4,  pnlMag5,  pnlMag6,  pnlMag7,
                           pnlMag8,   pnlMag9,  pnlMag10, pnlMag11, pnlMag12, pnlMag13, pnlMag14,
                           pnlAuto4,  pnlAuto5, pnlAuto6,
                           pnlFix7,   pnlFix8,  pnlFix9,  pnlFix10, pnlFix11, pnlFix12};  //JerryYang 20220909 : add magazine

    TColor ColorMap[]={clGray, clRed, clGreen, (TColor)0x000080FF, clBlack};
    int iColor, iBin;
    AnsiString sTempEng="", sTempChi="";
    static bool bChangeColor=false;
    static int iAlarmFlag=0;
    bool bHasError=false, bErrFlag[eBinDispTotal];                              //JerryYang 20220909 : 12->eBinDispTotal

    if(InitialOK==false)
    {
        return;
    }

    for(int i=0; i<eBinDispTotal; i++)                                          //JerryYang 20220909 : 12->eBinDispTotal
    {
        // GATE (D1): golden `if(HSys.BinDisCtrl->UnitHasInstall(i)) bErrFlag[i]
        // =HSys.BinDisCtrl->GerErrNow(i); else bErrFlag[i]=false;` -- see
        // forms/fShowBinSelect.h GATE REGISTER (D1). Substituted with
        // golden's own "not installed" outcome, true for every unit in this
        // port.
        bErrFlag[i]=false;
    }
    (void)bErrFlag;   // GATE (D2): its only READ (the final else-if condition below) is gated whole
    sTempChi="";                                                                //Jimmychiu 20231031 : add Bin display error record
    for(int i=0; i<eBinDispTotal; i++)                                          //JerryYang 20220909 : 12->eBinDispTotal
    {
        if(AUTO3_IS_MAGAZINE==0 && i>=eBinDispMag1 && i<=eBinDispMag14)         //JerryYang 20230515 : 沒有Magazine就不要顯示
        {
        }
        else if(AUTO3_IS_MAGAZINE==1 && i==eBinDispAuto3)
        {
        }
        else if(i==eBinDispBulkBox ||                                           //Jimmychiu 20231030 : bin Box not display
                i>=eBinDispAuto4)
        {
        }
        else if(TrayForm.bEnableAMR==true && (i>=eBinDispLoader && i<=eBinDispAuto2)) //Eastsun 20260515 F019 AMR hide BinDisplay
        {
        }
        else
        {
            // GATE (D2): golden `if(bErrFlag[i] || (i>=3 && HSys.BinDisCtrl->
            // UnitHasInstall(i)==false)) { bHasError=true; sTempChi=sTempChi
            // +" "+UnLoadPanel[i]->Name; if(iAlarmFlag==0) iAlarmFlag=1; }` --
            // see forms/fShowBinSelect.h GATE REGISTER (D2). Gated whole (not
            // substituted -- see that entry for why a fabricated value would
            // be wrong here).
        }
    }

    if(bHasError)
    {
        bChangeColor=!bChangeColor;
        (void)(PageControl1->ActivePageIndex==3);                              //tsUnloadMap -- GOLDEN ODDITY: `==` not `=`, a no-op comparison (golden :272); kept verbatim per translation-fidelity policy, see forms/fShowBinSelect.h WAVE D SCOPE. (void) cast avoids -Wunused-value without changing the (already no-op) behaviour.
    }
    else
    {
        iAlarmFlag=0;                                                           //Steven 20211130 : JSCC要求Bin顯示器異常要alarm
    }

    if(IniConfig.bG16BinDispNeedAlarm)                                          //Steven 20211220 : 修正G16, Bin顯示器異常要alarm
    {
        ;
    }
    else if(PageControl1->ActivePage!=tsUnloadMap)
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_AMD_M)               //Ifor 20221026 add: KYEC要求一直顯示
        {
        }
        else
        {
            return;
        }
    }

    for(int i=0; i<eBinDispTotal; i++)                                          //JerryYang 20220909 : 12->eBinDispTotal
    {
        // GATE (D3): golden's `if(HSys.BinDisCtrl->UnitHasInstall(i))`
        // true-arm (GetColorNow/GetBinNow + Caption) is unreachable in this
        // port -- see forms/fShowBinSelect.h GATE REGISTER (D3). Golden's own
        // ELSE arm below is real/ACTIVE and is this port's correct outcome
        // for every slot.
        UnLoadPanel[i]->Color=clGray;
        UnLoadPanel[i]->Caption="X";
    }
    (void)iColor; (void)iBin; (void)ColorMap;   // GATE (D3): only ever read/assigned inside the now-unreachable true-arm above

    if((CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_AMD_M) && bBinDispAlarm==false)//Ifor 20220714 add:Bin Disp 異常報警 每次Onecycle 檢查一次
    {
        if(bHasError && fHome->fShow==false)
        {
            bBinDispAlarm=true;
            #ifndef SOFT_SIMULTE
            // GATE (D4): golden `HSys.BinDisCtrl->CommBin->StopComm();
            // HSys.BinDisCtrl->bFirstInit=true; HSys.BinDisCtrl->
            // ProcessStopStart(true); ShowMyMessage("Please check bin
            // display. It have communication error!", "請確認Bin顯示器狀態。");`
            // -- see forms/fShowBinSelect.h GATE REGISTER (D4). Currently
            // dormant (bHasError is always false, GATE (D2)) but gated
            // regardless -- defense in depth against a direct BinDisCtrl
            // deref.
            #endif
        }
    }
    else if(IniConfig.bG16BinDispNeedAlarm)                                          //Steven 20211130 : JSCC要求Bin顯示器異常要alarm
    {
        if(iAlarmFlag==1)
        {
            AlarmDelay.SetSecAndOn(60);
            iAlarmFlag=2;
        }

        if(iAlarmFlag==2 && AlarmDelay.Off())
        {
            #ifndef SOFT_SIMULTE
            // GATE (D5): golden `sTempEng=AnsiString().sprintf("%s Error
            // part:%s", "Please check bin display. It have communication
            // error!", sTempChi); sTempChi=AnsiString().sprintf("%s 錯誤位置:
            // %s", "請確認Bin顯示器狀態!", sTempChi); HSys.BinDisCtrl->CommBin->
            // StopComm(); HSys.BinDisCtrl->bFirstInit=true; HSys.BinDisCtrl->
            // ProcessStopStart(true); ShowMyMessage(sTempEng,sTempChi);
            // AlarmDelay.SetSecAndOn(60); iAlarmFlag=0;` -- see forms/
            // fShowBinSelect.h GATE REGISTER (D5). Currently dormant
            // (iAlarmFlag is only ever set 1 inside GATE (D2)'s gated arm).
            #endif
        }
    }

    if(bHasError)
    {
        sbRunStatus->Panels->Items[0]->Text="Bin display got error!!";
        sbRunStatus->Color=clRed;
    }
    else
    {
        // GATE (D6): golden `HSys.BinDisCtrl->GetRunStatus()` -- see forms/
        // fShowBinSelect.h GATE REGISTER (D6). Safe-default substitute: "".
        sbRunStatus->Panels->Items[0]->Text="";
        sbRunStatus->Color=clBtnFace;
    }
}
//---------------------------------------------------------------------------
TQPF_Timer BinTimer;   // golden :997 -- file-scope global (NOT a class member), matches golden exactly
void TfShowBinSelect::DoShowBinDigital()
{
    if(bUpdateBinDigital==true &&
       (NUMBER_PANEL_TYPE==3 ||                                                 //Steven : 純顯示器
        NUMBER_PANEL_TYPE==4))                                                  //Sam 20240604 : 新增 BinDisplay TFT
    {
        int Data;//, iBinSelCT=0;
        int iBinSet[MAX_BIN_UNIT][TEST_MAX_BIN];                                //iBinSet[0~2][20] 是Loader, Empty, Color，所以不設定     //Steven 20140402 : Fixed 記憶體亂寫
        int iBinCount[TEST_MAX_BIN]={-1};                                       //每個位置被分配到幾個Bin                                 //Steven 20140402 : Fixed 記憶體亂寫
        int iBinColor[TEST_MAX_BIN]={-1};                                       //1:red, 2:green, 3:orange                                //Steven 20140402 : Fixed 記憶體亂寫

        //初始化
        for(int i=0; i<MAX_BIN_UNIT; i++)                                       //Steven 20140402 : Fixed 記憶體亂寫
        {
            iBinColor[i]=ColorOrange;
            iBinCount[i]=0;
            for(int j=0; j<TEST_MAX_BIN; j++)                                   //Steven 20140402 : Fixed 記憶體亂寫
                iBinSet[i][j]=-1;
        }

        AddBinDisp[eBinDispLoader    ]=eBinDispLoader;
        AddBinDisp[eBinDispEmpty     ]=eBinDispEmpty;
        AddBinDisp[eBinDispColor     ]=eBinDispColor;
        AddBinDisp[eBinDispAuto1     ]=eBinDispAuto1;
        AddBinDisp[eBinDispAuto2     ]=eBinDispAuto2;
        AddBinDisp[eBinDispAuto3     ]=eBinDispAuto3;
        AddBinDisp[eBinDispFix1      ]=eBinDispFix1 ;
        AddBinDisp[eBinDispFix2      ]=eBinDispFix2 ;
        AddBinDisp[eBinDispFix3      ]=eBinDispFix3 ;
        AddBinDisp[eBinDispFix4      ]=eBinDispFix4 ;
        AddBinDisp[eBinDispFix5      ]=eBinDispFix5 ;
        AddBinDisp[eBinDispFix6      ]=eBinDispFix6 ;
        AddBinDisp[eBinDispBulkBox   ]=eBinDispBulkBox;
        AddBinDisp[eBinDispMag1      ]=eBinDispMag1;
        AddBinDisp[eBinDispMag2      ]=eBinDispMag2;
        AddBinDisp[eBinDispMag3      ]=eBinDispMag3;
        AddBinDisp[eBinDispMag4      ]=eBinDispMag4;
        AddBinDisp[eBinDispMag5      ]=eBinDispMag5;
        AddBinDisp[eBinDispMag6      ]=eBinDispMag6;
        AddBinDisp[eBinDispMag7      ]=eBinDispMag7;
        AddBinDisp[eBinDispMag8      ]=eBinDispMag8;
        AddBinDisp[eBinDispMag9      ]=eBinDispMag9;
        AddBinDisp[eBinDispMag10     ]=eBinDispMag10;
        AddBinDisp[eBinDispMag11     ]=eBinDispMag11;
        AddBinDisp[eBinDispMag12     ]=eBinDispMag12;
        AddBinDisp[eBinDispMag13     ]=eBinDispMag13;
        AddBinDisp[eBinDispMag14     ]=eBinDispMag14;
        AddBinDisp[eBinDispAuto4     ]=eBinDispAuto4;
        AddBinDisp[eBinDispAuto5     ]=eBinDispAuto5;
        AddBinDisp[eBinDispAuto6     ]=eBinDispAuto6;
        AddBinDisp[eBinDispFix7      ]=eBinDispFix7;
        AddBinDisp[eBinDispFix8      ]=eBinDispFix8;
        AddBinDisp[eBinDispFix9      ]=eBinDispFix9;
        AddBinDisp[eBinDispFix10     ]=eBinDispFix10;
        AddBinDisp[eBinDispFix11     ]=eBinDispFix11;
        AddBinDisp[eBinDispFix12     ]=eBinDispFix12;

        if(AUTO_EMPTY_COLOR>=3)                                                 //Fix 1-6 Addr從0開始, 使用第二組COM PORT
        {
            AddBinDisp[eBinDispFix1  ]=0;
            AddBinDisp[eBinDispFix2  ]=1;
            AddBinDisp[eBinDispFix3  ]=2;
            AddBinDisp[eBinDispFix4  ]=3;
            AddBinDisp[eBinDispFix5  ]=4;
            AddBinDisp[eBinDispFix6  ]=5;
            AddBinDisp[eBinDispFix7  ]=6;
            AddBinDisp[eBinDispFix8  ]=7;
            AddBinDisp[eBinDispFix9  ]=8;
            AddBinDisp[eBinDispFix10 ]=9;
            AddBinDisp[eBinDispFix11 ]=10;
            AddBinDisp[eBinDispFix12 ]=11;

            AddBinDisp[eBinDispAuto4 ]=3+eAuto4;                                // L=0 E=1 C=2 A1=3 A2=4 A3=5 A4=6 A5=7 A6=8
            AddBinDisp[eBinDispAuto5 ]=3+eAuto5;
            AddBinDisp[eBinDispAuto6 ]=3+eAuto6;
        }

        if(TestIF_File.iMagDisplayOrder==1)
        {
            AddBinDisp[eBinDispMag1      ]=1;                                   //BIN顯示器有做15顆, addr 0不使用,從addr1開始
            AddBinDisp[eBinDispMag2      ]=2;
            AddBinDisp[eBinDispMag3      ]=3;
            AddBinDisp[eBinDispMag4      ]=4;
            AddBinDisp[eBinDispMag5      ]=5;
            AddBinDisp[eBinDispMag6      ]=6;
            AddBinDisp[eBinDispMag7      ]=7;
            AddBinDisp[eBinDispMag8      ]=8;
            AddBinDisp[eBinDispMag9      ]=9;
            AddBinDisp[eBinDispMag10     ]=10;
            AddBinDisp[eBinDispMag11     ]=11;
            AddBinDisp[eBinDispMag12     ]=12;
            AddBinDisp[eBinDispMag13     ]=13;
            AddBinDisp[eBinDispMag14     ]=14;
        }
        else
        {
            AddBinDisp[eBinDispMag1      ]=14;                                  //JerryYang 20250919 : Magazine 1在最下面, addr是14
            AddBinDisp[eBinDispMag2      ]=13;
            AddBinDisp[eBinDispMag3      ]=12;
            AddBinDisp[eBinDispMag4      ]=11;
            AddBinDisp[eBinDispMag5      ]=10;
            AddBinDisp[eBinDispMag6      ]=9;
            AddBinDisp[eBinDispMag7      ]=8;
            AddBinDisp[eBinDispMag8      ]=7;
            AddBinDisp[eBinDispMag9      ]=6;
            AddBinDisp[eBinDispMag10     ]=5;
            AddBinDisp[eBinDispMag11     ]=4;
            AddBinDisp[eBinDispMag12     ]=3;
            AddBinDisp[eBinDispMag13     ]=2;
            AddBinDisp[eBinDispMag14     ]=1;
        }

        iBinSet[0][0]=111;  //Loader
        iBinSet[1][0]=104;  //Empty Tray
        iBinSet[2][0]=102;  //Color Tray
        iBinSet[0][1]=-1;   //Loader
        iBinSet[1][1]=-1;   //Empty Tray
        iBinSet[2][1]=-1;   //Color Tray
        iBinColor[0]=ColorOrange;     //橘色
        iBinColor[1]=ColorOrange;
        iBinColor[2]=ColorOrange;

        //設定Bin的位置
        for(int i=0; i<iTestBinCount; i++)                                      //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
        {
            Data=iTo3PosUnload[Prod.iT6PosCate[i]];                             //JerryYang 20230926
            if(Data<=0 || Data>e3TrayCount) continue;                           //JerryYang 20220909 : 修正是一    //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
            for(int j=1; j<=e3TrayCount; j++)                                   //RogerYang 20250825 : "<" -> "<=" 修正最後一顆顯示器沒寫入 //Steven 20140404 Start : Fixed 256Bin
            {
                if(j==Data)                                                     //Steven 20140402 : Data-1 --> Data
                {
                    iBinSet[Data+2][iBinCount[Data+2]]=i;                       //Steven 20140402 : i+1 --> i
                    iBinCount[Data+2]++;
                }
            }
        }

        if(IniConfig.bAutoTrayLink==true)                                       //jou 2012-06-14 Auto Tray Link
        {
            if(Prod.bLinkTo6Tray[1]==true)
            {
                iBinCount[4]=iBinCount[3];
                for(int i=0; i<MAX_BIN_UNIT; i++)                               //Steven 20140402 : Fixed 記憶體亂寫
                    iBinSet[4][i]=iBinSet[3][i];
            }

            if(Prod.bLinkTo6Tray[1]==true && Prod.bLinkTo6Tray[2]==true)
            {
                iBinCount[5]=iBinCount[3];
                for(int i=0; i<MAX_BIN_UNIT; i++)                               //Steven 20140402 : Fixed 記憶體亂寫
                    iBinSet[5][i]=iBinSet[3][i];
            }
            else if(Prod.bLinkTo6Tray[1]==false && Prod.bLinkTo6Tray[2]==true)
            {
                iBinCount[5]=iBinCount[4];
                for(int i=0; i<MAX_BIN_UNIT; i++)                               //Steven 20140402 : Fixed 記憶體亂寫
                    iBinSet[5][i]=iBinSet[4][i];
            }
        }

        //Steven 20181113 : 修正Fix Link顯示問題
        //==>
        if(Prod.bLinkTo6Tray[eFix2]==true)
        {
            iBinCount[7]=iBinCount[6];
            for(int i=0; i<MAX_BIN_UNIT; i++)                                   //Steven 20140402 : Fixed 記憶體亂寫
                iBinSet[7][i]=iBinSet[6][i];
        }

        if(Prod.bLinkTo6Tray[eFix2]==true && Prod.bLinkTo6Tray[eFix3]==true)
        {
            iBinCount[8]=iBinCount[6];
            for(int i=0; i<MAX_BIN_UNIT; i++)                                   //Steven 20140402 : Fixed 記憶體亂寫
                iBinSet[8][i]=iBinSet[6][i];
        }
        else if(Prod.bLinkTo6Tray[eFix2]==false && Prod.bLinkTo6Tray[eFix3]==true)
        {
            iBinCount[8]=iBinCount[7];
            for(int i=0; i<MAX_BIN_UNIT; i++)                                   //Steven 20140402 : Fixed 記憶體亂寫
                iBinSet[8][i]=iBinSet[7][i];
        }
        //<==
        //Steven 20181113 : 修正Fix Link顯示問題
        //Ifor 20231122 add Magazine Link
        //==>
        if(AUTO3_IS_MAGAZINE==1)
        {
            for(int i=0; i<14; i++)
            {
                if(BinSelect[iTestRunMode].bMagazineLink[i]==true)
                {
                    for(int j=0; j<14; j++)
                    {
                        // GOLDEN ODDITY (same shape as GATE (B12)'s second
                        // note, ShowBinSel's own Magazine-Link inner loop):
                        // a pathological all-true bMagazineLink[] chain down
                        // to i==j would walk `bMagazineLink[i-j]` to index 0,
                        // still in-bounds, but a chain reaching PAST index 0
                        // is unreachable in practice (index 0/eAuto1 slot is
                        // never written true by any port'd writer) -- kept
                        // verbatim.
                        if(BinSelect[iTestRunMode].bMagazineLink[i-j]==false)
                        {
                            iBinCount[13+i]=iBinCount[13+i-j];
                            for(int k=0; k<MAX_BIN_UNIT; k++)
                                iBinSet[13+i][k]=iBinSet[13+i-j][k];
                            break;
                        }
                    }
                }
            }
        }
        //<==
        //Ifor 20231122 add Magazine Link
        if(TestIF_File.bEnableQASampling && TestIF_File.iQASamplingT3Pos!=0)     //Steven 20190326 : QA Sampling
        {
            iBinSet[TestIF_File.iQASamplingT3Pos-1][0]=116;
        }

        //設定Error Bin的位置
        Data=iTo3Unload[Prod.iIfErrorT6];
        if(Data>=0 && Data<e3TrayCount)                                         //JerryYang 20220909 : 9->iBinSelCT
        {
            if(Data>=eBinDispMag1 && Data<=eBinDispMag14)                       //JerryYang 20220909 : add magazine
            {
                if(AUTO3_IS_MAGAZINE==1)
                {
                   int iSort=Data-9;
                   int iLinkCount=0;    //Ifor 20240909 Fix:記錄鍊條的位置
                    for(int i=iSort; i<14; i++)
                    {
                        if(BinSelect[iTestRunMode].bMagazineLink[i]==true)
                        {
                            iLinkCount++;
                            iBinSet[Data+iLinkCount+3][iBinCount[Data+iLinkCount+3]]=999;    //Magazine bin不支援錯誤顯示, 999當成ERROR BIN
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                iBinSet[Data+3][iBinCount[Data+3]]=999;                         //Magazine bin不支援錯誤顯示, 999當成ERROR BIN
            }
            else
            {
                iBinSet[Data+3][iBinCount[Data+3]]=104;
            }
            iBinCount[Data+3]++;
        }

        //設定顏色
        for(int j=0; j<eTrayCount; j++)
        {
            if(Prod.iIsFailT6[j]>0)                                             //Steven 20240105 : Prod.bIsPass --> Prod.iIsFailT6   //Steven 20240709 : ==1 --> >0
                iBinColor[iTo3Unload[j]+3]=ColorRed;
            else
                iBinColor[iTo3Unload[j]+3]=ColorGreen;
        }

        //Steven 20181113 : 修正Fix Link顯示問題
        //==>
        if(Prod.bLinkTo6Tray[eFix2]==true)
        {
            iBinColor[7]=iBinColor[6];
        }

        if(Prod.bLinkTo6Tray[eFix2]==true && Prod.bLinkTo6Tray[eFix3]==true)
        {
            iBinColor[8]=iBinColor[6];
        }
        else if(Prod.bLinkTo6Tray[eFix2]==false && Prod.bLinkTo6Tray[eFix3]==true)
        {
            iBinColor[8]=iBinColor[7];
        }
        //<==
        //Steven 20181113 : 修正Fix Link顯示問題

        for(int i=3; i<MAX_BIN_UNIT; i++)                                       //Steven 20140402 : Fixed 記憶體亂寫
            if(iBinSet[i][0]==-1)
                iBinColor[i]=ColorOrange;                                       //沒用到的就顯示橘色

        //jou 2014-03-28 SPIL Handler  On-line & Offline Switch Flow
        if(IniConfig.bSPILFunction==true && LastSet.iTester==OFF_LINE)          //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
        {
            for(int i=0; i<MAX_BIN_UNIT; i++)
            {
                iBinColor[i]=ColorRed;
                iBinCount[i]=2;
                for(int j=0; j<MAX_BIN_UNIT; j++)
                {
                    if(j==0)
                        iBinSet[i][j]=123;
                    else if(j==1)
                        iBinSet[i][j]=0;
                    else
                        iBinSet[i][j]=-1;
                }
            }
        }

        //開始設定顯示器上
        for(int i=0; i<MAX_BIN_UNIT; i++)                                       //Steven 20140402 : Fixed 記憶體亂寫
        {
            //==> Eastsun 20260513 KYEC Auto change tray flash guard
            if(IniConfig.bP66AutoChangingFlashWarn)
            {
                if(i==eBinDispAuto1 && bAutoChangingWarn[0]) continue;
                if(i==eBinDispAuto2 && bAutoChangingWarn[1]) continue;
                if(i==eBinDispAuto3 && bAutoChangingWarn[2]) continue;
            }
            //<== Eastsun 20260513
            // GATE (D7): golden `HSys.BinDisCtrl->WriteTargetBin(i,
            // iBinSet[i], iBinColor[i]);` -- see forms/fShowBinSelect.h GATE
            // REGISTER (D7). iBinSet[][]/iBinColor[] above are computed IN
            // FULL regardless (only this sink is gated).
        }

        bUpdateBinDigital=false;

        // GATE (D8): golden `HSys.BinDisCtrl->ProcessStopStart(true);` --
        // see forms/fShowBinSelect.h GATE REGISTER (D8).
        return;
    }

    if(NUMBER_PANEL_TYPE==0)
        return;                                                                 //Steven 20100512 : 沒有安裝
    if(bUpdateBinDigital==false)
        return;

    int &Task=iShowBinDigitalTask;
    bool bFlag;
    static int iCount=0;
    switch(Task)
    {
        case 1:
            // GATE (D9): golden `if(fiosetview->fShow) { SW[iNumPanelDown]
            // .On(); Task=400; } else {...}` -- see forms/fShowBinSelect.h
            // GATE REGISTER (D9). Forced false; golden's own ELSE arm below
            // is real/ACTIVE and needs no unavailable facade.
            {
                for(int i=0; i<12; i++)
                {
                    SW[SwLoaderBin+i].On();
                }
                iCount=0;
                Task=100;
            }
            ShowBinDigital();                                                  // AI(W906-FW3-SBS-WE) 20260820: GATE (D10) RETIRED -- real body now below (was: documented no-op stub)
            BinTimer.SetSecAndOn(2);
            break;
        case 100:
            if(BinTimer.Off())
            {
                for(int i=0; i<12; i++)
                {
                    SW[SwLoaderBin+i].Off();
                }
                Task=200;
                BinTimer.SetSecAndOn(0.1);
            }
            break;
        case 200:
            if(BinTimer.Off())
            {
                iCount++;
                bFlag=false;
                for(int i=0; i<12; i++)
                {
                    if(iCount<=iShowAutoBin[i])
                    {
                        SW[SwLoaderBin+i].On();
                        bFlag=true;
                    }
                }
                BinTimer.SetSecAndOn(0.1);
                Task=100;
                if(bFlag==false)
                {
                    Task=300;
                }
            }
            break;
        case 300:
            bUpdateBinDigital=false;                                            //Steven 20260612 : Fix == to = (was comparison, not assignment)
            break;
        case 400:                                                               //Steven 20090919 Start
            // GATE (D11): unreachable in this port (GATE (D9) forces case 1
            // to always take the else-arm, so Task can never become 400) --
            // body also needs iNumPanelDown/bNumPanelDown/fiosetview->fShow,
            // none of which have a facade or a true home in this wave's
            // write boundary. See forms/fShowBinSelect.h GATE REGISTER (D11).
            break;
        case 500:
            // GATE (D11): same reasoning as case 400.
            break;
    }
}
//---------------------------------------------------------------------------
// AI(W906-FW3-SBS-WE) 20260820: GATE (D10) RETIRED -- ShowBinDigital (golden
// :880-996) was left as a documented no-op stub by WAVE D (see forms/
// fShowBinSelect.h WAVE D SCOPE / former GATE (D10)); this wave gives it its
// real body. WAVE D's own recon note already corrected the record on what
// this function actually touches (pure iShowAutoBin[]/MyBinSel[]/Prod
// arithmetic, ZERO SW[]/fiosetview) -- re-verified again this wave (`grep -n
// "SW\[\|fiosetview"` restricted to the cp950-decoded golden :880-996 span,
// 0 hits), and it held up: no BinDisCtrl deref, no disk write, no motor/IO
// call anywhere in this function, so it needs NO new GATE of its own. Golden
// ref: cShowBinSelect.cpp golden :880-996, cp950-decoded this wave
// (`python3 -c "open(path,'rb').read().decode('cp950')"`), 0 U+FFFD over the
// decoded span (re-verified 20260820).
//---------------------------------------------------------------------------
void TfShowBinSelect::ShowBinDigital()
{
    if(NUMBER_PANEL_TYPE==3 ||
       (NUMBER_PANEL_TYPE==4 && MAGAZINE_BIN_DISP_TYPE!=eMagBinUninstall))      //Sam 20240604 : 新增 BinDisplay TFT

        return;

    if(NUMBER_PANEL_TYPE!=0)                                                    //Steven 20091001
    {
        int i, j;
        int iArray[12];
        //Steven 20090917 Start
        for(i=0; i<12; i++)
            iArray[i]=-1;
        if(NUMBER_PANEL_TYPE==2)        //2 Digital
        {
            iArray[0]=111;     //L
            iArray[1]=104;     //E
            iArray[2]=102;     //C
        }
        else if(NUMBER_PANEL_TYPE==1)   //1 Digital
        {
            iArray[0]=21;      //L
            iArray[1]=14;      //E
            iArray[2]=12;      //C
        }
        //Steven 20090917 End
        for(i=0; i<12; i++)
            iShowAutoBin[i]=iArray[i];
        for(i=3; i<12; i++)
        {
            for(j=0; j<iTestBinCount; j++)
            {
                if(iShowAutoBin[i]==-1)
                {
                    if(Prod.iT6PosCate[j]==(i-2))
                       // iShowAutoBin[i]=j+1;
                        iShowAutoBin[i]=j;//kevin 20140324 bin 0
                }
            }
        }

        if(IniConfig.bAutoTrayLink==true)                                       //jou 2012-06-14 Auto Tray Link
        {
            if(Prod.bLinkTo6Tray[1]==true)
            {
                iShowAutoBin[4]=iShowAutoBin[3];
            }

            if(Prod.bLinkTo6Tray[1]==true && Prod.bLinkTo6Tray[2]==true)
            {
                iShowAutoBin[5]=iShowAutoBin[3];
            }
            else if(Prod.bLinkTo6Tray[1]==false && Prod.bLinkTo6Tray[2]==true)
            {
                iShowAutoBin[5]=iShowAutoBin[4];
            }
        }

        //Steven 20181113 : 修正Fix Link顯示問題
        //==>
        if(Prod.bLinkTo6Tray[eFix2]==true)
        {
            iShowAutoBin[7]=iShowAutoBin[6];
        }

        if(Prod.bLinkTo6Tray[eFix2]==true && Prod.bLinkTo6Tray[eFix3]==true)
        {
            iShowAutoBin[8]=iShowAutoBin[6];
        }
        else if(Prod.bLinkTo6Tray[eFix2]==false && Prod.bLinkTo6Tray[eFix3]==true)
        {
            iShowAutoBin[8]=iShowAutoBin[7];
        }
        //<==
        //Steven 20181113 : 修正Fix Link顯示問題

        //Ifor 20231122 add Magazine Link
        //==>
        if(AUTO3_IS_MAGAZINE==1)
        {
            for(i=0; i<14; i++)
            {
                if(BinSelect[iTestRunMode].bMagazineLink[i]==true)
                {
                    for(j=0; j<14; j++)
                    {
                        // AI(W906-FW3-SBS-WE) 20260820, GOLDEN NOTE (same
                        // negative-subscript shape as this file's OWN
                        // cShowBinSelect.cpp:1325-1332 GOLDEN NOTE, in
                        // ShowBinSel's sibling AUTO3_IS_MAGAZINE block): this
                        // outer loop starts at i=0 (not i=1 like the
                        // sibling), so taken alone the i==0 iteration would
                        // already index bMagazineLink[i-j]==bMagazineLink[-1]
                        // at j==1 if bMagazineLink[0] alone were true. In
                        // practice the outer `if` above never even enters for
                        // i==0, because bMagazineLink[0] (the exact same slot
                        // the sibling note documents, written only by
                        // cBinSel.cpp:1722) is never set true by any writer
                        // this tree ports -- so this loop behaves as if it
                        // started at i=1 in practice, same invariant as the
                        // sibling, kept verbatim.
                        if(BinSelect[iTestRunMode].bMagazineLink[i-j]==false)
                        {
                            MyBinSel[eMag1+i]->Caption=MyBinSel[eMag1+(i-j)]->Caption;
                            break;
                        }
                    }
                }
                else
                {
                    // AI(W906-FW3-SBS-WE) 20260820, GOLDEN ODDITY:
                    // `MyBinSel[eMag1+i]->Caption=MyBinSel[eMag1+i]->
                    // Caption;` below is a redundant self-assignment (a=a),
                    // a harmless no-op. Reads as an explicit "not linked ->
                    // leave Caption unchanged" else-arm rather than a typo
                    // (unlike GATE (B12)'s `==` comparison-statement in
                    // ChangeBinDispStatus) -- kept verbatim, including
                    // golden's own extra leading space on this one line.
                     MyBinSel[eMag1+i]->Caption=MyBinSel[eMag1+i]->Caption;
                }
            }
        }
        //<==
        //Ifor 20231122 add Magazine Link

        //jou 2010-01-12 start : E bin show "E"   (E=R)
        if(CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_ASE_KaohSiung_K12)     //Steven 20131101 : Add ASE-K12
        {
            iShowAutoBin[Prod.iIfErrorT6+3]=104; //E
        }
        else
        {
            //Steven 20100224 無法顯示"R"
            //iShowAutoBin[Prod.iIfErrorT6+3]=117; //R
        }
        //jou 2010-01-12 end
    }
}
