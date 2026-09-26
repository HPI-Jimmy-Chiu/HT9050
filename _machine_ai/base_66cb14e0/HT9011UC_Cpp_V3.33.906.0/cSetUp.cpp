// =============================================================================
//  cSetUp.cpp  --  Setup-dialog logic that needs ht9045_sm-layer symbols
//                  (site-map suck-mode reload, STM close-site dispatch, the
//                  Auto-Site-Mapping checkbox guard).
//
//  AI(W906-FW-SETUP-B) 20260821: new file. Translation wave: FW-SETUP-B
//  (cSetUp Wave B, form-logic campaign). Golden source of truth:
//  HT9011UC_Code_V3.33.906.0_20260618/cSetUp.cpp (4,858 lines, cp950 --
//  decoded with `python3 -c "open(p,'rb').read().decode('cp950')"`, 0
//  U+FFFD, 20260821) + cSetUp.h (295 lines) + cSetUp.dfm (cp950, 0 U+FFFD).
//
//  WHY THIS FILE EXISTS SEPARATELY FROM forms/fSetup.cpp -- read forms/
//  fSetup.h's Wave B banner FIRST (full citation there, not repeated in
//  full here): forms/fSetup.cpp is compiled into the `ht9045_forms` CMake
//  target, which CMakeLists.txt:706 restricts to `vclcompat ht9045_globals`
//  only -- a deliberate bottom-layer diet, and CMakeLists.txt:589-593
//  documents that `ht9045_forms PUBLIC ht9045_sm` does not even CONFIGURE
//  (a real cycle: ht9045_sm already PUBLIC-links ht9045_forms,
//  CMakeLists.txt:2216). The 3 methods below all need real, non-stub bodies
//  that live in `ht9045_sm` (aHotPlateSubstrate.cpp/csystem.cpp/ainarm2.cpp/
//  ainarm9045_2x6_8.cpp/ainarm9045_2x8_8.cpp/ainarm9045_1x4_4.cpp/
//  ainarm9045_1x3_2_14.cpp/canary_support.cpp), so this file is written to
//  belong to `ht9045_sm` -- the SAME split cTemperFrom.cpp/MainTempMode.cpp
//  already established for TfTemperFrom::/TfMain:: methods with the same
//  shape of dependency (root .cpp for the ht9045_sm-heavy methods; the
//  forms/fXxx.cpp sibling stays on the ht9045_globals-only diet).
//
//  CMAKE STATUS -- WIRED into ht9045_sm (CMakeLists.txt:1922) by the
//  FW-SETUP-B integration pass, 20260821.
//  AI(W906-FW-SETUP-D-integ) 20260824: the paragraph that stood here
//  ("NOT YET WIRED ... UNREACHABLE") was the Wave B agent's own edit-boundary
//  note and went stale the same day it was written -- and it then misled the
//  Wave D agent into re-reporting the file as unwired. Everything in this
//  file is LIVE code in the ht9045_sm archive; new methods compile into every
//  build, so wave acceptance for this file is a real build, not syntax-only.
//
//  WAVE SCOPE -- 3 of 47 golden TfSetup:: methods (all (a) 顯示/讀取側,
//  read line-by-line in full):
//    ReadUseSuckModeFile   golden :2133-2146 (14L) -- reloads TestIF_File.
//      iUseSuckMode from the recipe file, then either lets
//      AdjustShuttlePlaceOrder run (via ResetShuttleWhichKit) or defers to
//      ChangeHotPlateData if IC is present. THE method MainTempMode.cpp's
//      two SAFETY GATEs (dep-fSetup-ReadUseSuckModeFile, golden :21886/
//      :21893) are waiting on -- see the READ/WRITE AUDIT section below for
//      why it is safe to land un-gated.
//    CheckSTMMode          golden :2148-2177 (30L) -- pure dispatch: resets
//      3 close-site-mode globals to their Standard values, then calls AT
//      MOST one of 5 already-real per-arm-variant functions based on
//      TestIF_File.iTestMode/iInArmType. Zero widget dependency.
//    cbI21Click            golden :4804-4828 (25L) -- the "Enable auto site
//      mapping function" checkbox's guard: refuses to let the operator
//      manually close Auto-Site-Mapping while there is still IC in the
//      machine (re-checks it, re-sets the checkbox back on, and warns).
//  Total: 3 methods / 69 golden lines this file. Denominator: TfSetup:: has
//  47 distinct methods tree-wide (same count Wave A verified,
//  forms/fSetup.h's own banner); 18/47 now have real bodies (15 Wave A + 3
//  here); the other 29 are queued -- see this wave's hand-off report for the
//  per-method reason (write-path / ctor-array-blocked / TScrollBar-zero-port
//  / TImage-zero-port / fQwertyKey-gated / fPassword-unported / other
//  established GATEs, each cited against real golden line ranges read this
//  wave, not inferred from name/size alone).
//
//  READ/WRITE AUDIT -- ReadUseSuckModeFile (task-mandated, 20260821):
//    * `GetRecipeFileName("HandlerCondition.Data")` resolves to
//      `DataPath + GetLastOpenFN() + "HandlerCondition.Data"` (common.cpp:
//      2410-2414) -- the per-recipe RUNTIME DATA directory family, NOT
//      system\Gerneral.ini and NOT ProductionInfo's *.bin family. Distinct,
//      already-real, already-used-tree-wide path.
//    * `ReadIniData(FileName, Group, Name, int Default)` (common.cpp:722-732)
//      is a PURE READ: `INIFile->ReadInteger(Group, Name, Value)` with the
//      caller's Value as fallback on open failure. NO CheckAndReadIniData-
//      style missing-key write-back, no MyForceDirectories, no bare
//      WriteIniData anywhere in ReadUseSuckModeFile's own golden body
//      (grepped this wave: `ReadUseSuckModeFile`'s 14 golden lines contain
//      exactly one I/O call, the ReadIniData above).
//    * The ONE systemic risk that DOES apply here is not unique to this
//      function: `OpenIniFile` (common.cpp:432-443) keeps a single global
//      `TIniFile *INIFile`, and opening a DIFFERENT filename than the one
//      currently held calls `CloseIniFile()` first, which calls
//      `INIFile->UpdateFile()` (common.cpp:457) -- a FLUSH that reformats
//      whatever file was PREVIOUSLY open (strips comments/blank lines,
//      values unchanged; the same "TIniFile flush毀排版" shape as the
//      Gerneral.ini 20260817 incident memory). This is TRUE of every single
//      ReadIniData call site already active tree-wide (csystem.cpp alone has
//      59), not a NEW vector this function introduces -- ReadUseSuckModeFile
//      adds no new risk beyond what every other already-shipped ReadIniData
//      caller already carries.
//    * `ResetShuttleWhichKit()` (ainarm2.cpp:3971-3989, ACTIVE) is PURE
//      in-memory logic (calls AdjustShuttlePlaceOrder() or
//      ChangeHotPlateData() depending on LastSet.iTemperature/MOT[].HasIC())
//      -- no file I/O of its own.
//    CONCLUSION: ReadUseSuckModeFile has NO counter.clear-shape write risk,
//    no MyForceDirectories, no bare WriteIniData. It is safe to land
//    un-gated as a genuine (a) 顯示/讀取側 method. MainTempMode.cpp's two
//    gates are UNCHANGED this wave (MainTempMode.cpp is out of this wave's
//    edit boundary) -- whether/when the main loop un-gates them is now
//    blocked ONLY on (1) wiring this file into CMakeLists' ht9045_sm source
//    list (see CMAKE STATUS above) and (2) the scratch-redirect window the
//    task brief flagged for `LoadMachineConfig()`-adjacent tools -- NOT on
//    any further code gap in ReadUseSuckModeFile itself.
//
//  DEPENDENCY AUDIT (this wave, 20260821) -- every non-widget global/
//  function these 3 methods touch was grepped against this port tree before
//  translating; all resolve inside `ht9045_sm` or its own declared PUBLIC
//  dependencies (ht9045_globals directly; ht9045_core transitively via
//  ht9045_sm -> ht9045_secsgem -> ht9045_core, CMakeLists.txt:2209/:1165 --
//  the same path csystem.cpp's own 59 ReadIniData/GetRecipeFileName call
//  sites already rely on):
//    aHotPlateSubstrate.h  -- InArmSuck / OutArmSuck (TMyKitSuck) / MOT[] /
//                             ResetShuttleWhichKit / iCloseSiteModeFor2x6 /
//                             iCloseSiteModeFor2x8 / iCloseSiteModeFor1x4
//    csystem.h             -- IndexHasIC() / ShuttleHasIC()
//    canary_support.h      -- LastSet (-> LastSet.h, ht9045_globals) /
//                             ShowMyMessage
//  CheckSTMMode's 5 per-arm-variant dispatch targets + the e2x6Standard/
//  e1x4Standard enum values are declared LOCALLY below instead of by
//  including ainarm9045_2x6_8.h/ainarm9045_2x8_8.h/ainarm9045_1x4_4.h/
//  ainarm9045_1x3_2_14.h -- discovered this wave (g++ -fsyntax-only,
//  20260821): ainarm9045_2x6_8.h's `enum e2x6Mode` and ainarm9045_1x4_4.h's
//  `enum e1x4Mode` each redeclare an enumerator name
//  (e2x6OneByOne / e1x4CloseAbAc) that aHotPlateSubstrate.h ALREADY shims as
//  its own anonymous enum for unrelated callers (aHotPlateSubstrate.h:878/
//  :975) -- `enum { e2x6OneByOne = 2 };` vs `enum e2x6Mode { ...,
//  e2x6OneByOne=2, ... };` in the same TU is a hard redeclaration error, not
//  a mere warning. (ainarm9045_2x8_8.h/ainarm9045_1x3_2_14.h carry no such
//  clash and could be included safely today, but are left out too, for
//  consistency and so this file does not implicitly depend on
//  aHotPlateSubstrate.h never adding a similar shim for them later.) Same
//  "declare just the symbol needed" posture as NewRecordProcess below; all
//  5 functions are void() with no default arguments, so there is no
//  default-argument collision risk the way NewRecordProcess has.
//
//  NewRecordProcess -- declared locally with the SAME signature Command.cpp
//  (:282) and MainTempMode.cpp (:57) already declare it with, for the SAME
//  reason documented at both sites: including cMyDB.h or acatchtray_shims.h
//  here would reintroduce THEIR OWN NewRecordProcess declaration, which
//  collides on default arguments in the same TU. Real body today is the
//  logging-sink stub at acatchtray_shims.cpp:145 (MainTempMode.cpp's own
//  banner already documents this as the accepted current behaviour, not a
//  gap introduced here).
// =============================================================================
// =============================================================================
//  AI(W906-FW-SETUP-D) 20260824: Wave D -- the ctor-array / TScrollBar unlock
//  wave (appended to this banner region; everything above is verbatim Wave B).
//  Golden re-decoded this wave: cSetUp.cpp 4,858 lines, cp950, 0 U+FFFD.
//
//  WHAT LANDS THIS WAVE (bodies appended at this file's tail):
//    * golden cSetUp.cpp:49-57's ctor-built control arrays as file-scope
//      globals (TestSiteCH[][]/TestLabCol[]/TestLabRow[]/iTestSiteCh[][]/
//      MyTempRGBox[]) -- ZERO users outside cSetUp.cpp in golden (walked all
//      golden .cpp/.h via python cp950 scan, 20260824: every one of the 5
//      names hits only cSetUp.cpp), so they stay file-scope here exactly as
//      in golden. The OTHER file-scope globals of that block (iASMTestMode/
//      iASMSiteMap :53-54, bNeedEnterPassword :55, sSigPassword :56,
//      OrgTestMode :121) are NOT defined this wave -- their only consumers
//      are still-queued methods (FormShow/cbAaChange/cbAaDropDown/
//      sbtExitClick); defining them now would be surface with no reader.
//      [FW-SETUP-E 20260824 update: bNeedEnterPassword/sSigPassword are now
//       defined at this file's tail (explicit static) -- cbAaDropDown landed;
//       iASMTestMode/iASMSiteMap/OrgTestMode remain undefined as stated.]
//    * TfSetup::Init() -- golden's ctor :123-216, per the fTemp_Set.h:483
//      "ctor only fields, real logic -> explicit Init()" convention (the
//      global `fSetup = new TfSetup()` in forms/fSetup.cpp:21 is a static
//      initializer; this body reads MachineTypeChoice/USE_IN_OUT_ARM_Y_PITCH
//      -- other TUs' globals -- so it must NOT run at static-init time).
//      NOT CALLED anywhere yet (same posture as TfTemp_Set::Init pre-wiring):
//      FormDestroy/CompChange/CHSetError/CoSocketComboChange below dereference
//      what Init() builds, so the future wiring wave must call Init() first
//      -- golden got that ordering for free from the VCL ctor.
//    * 6 more methods: FormDestroy/CompChange/CHSetError/rgUseSuckModeClick/
//      CheckShuttlePitch/CoSocketComboChange (spans in the ledger below).
//
//  GATE REGISTER -- Wave D additions (Wave B's audit sections above unchanged)
//  ---------------------------------------------------------------------------
//  (G-Align)    Init(): golden :213 `MyTempRGBox[i]->Align=alTop;` -- gated,
//      same gate forms/fTemp_Set.h's register established (no Align port on
//      any vclcompat control, by design: rendering = web).
//  (G-Delegate) Init(): golden :214 `MyTempRGBox[i]->OnClick=rgSensor1Click;`
//      -- gated, same gate forms/fTemp_Set.h's register established (stock
//      vclcompat widgets carry no assignable OnClick delegate slot; the
//      OnChange/OnScroll slots on the NEW vclcompat::TScrollBar are the
//      user-ruled 20260824 exception, deliberately confined to that type).
//  NO config-write gates were needed in any Wave D body: grepped each golden
//  span for WriteIniData/CheckAndReadIniData/SaveToFile/MyForceDirectories,
//  20260824 -- 0 hits in :123-216/:218-230/:1195-1262/:1284-1605/:3423-3450/
//  :4645-4706/:4752-4770 (the write-heavy members of this class are exactly
//  the still-queued ones, see ledger).
//
//  47-METHOD LEDGER (rebuilt this wave against golden, spans re-measured;
//  single authoritative copy -- forms/fSetup.h's banners cite this table)
//  ---------------------------------------------------------------------------
//  TRANSLATED 25/47 (15 Wave A, forms/fSetup.cpp; 3 Wave B + 7 Wave D, here):
//    A GetTestMode:2179-2189  A SetShtMode:4708-4715  A VertifyShtModeisDiff:4717-4730
//    A XPitchKeyPress:3403-3407  A RadioButton1KeyDown:4443-4449 (dfm-orphaned, see fSetup.h)
//    A rgSensor1Click:4772-4777  A rgShtModeNormalClick:1264-1282
//    A Arm1PickArm2TestClick:4464-4474  A cbUseSLKClampClick:4637-4643
//    A rgYPitchOffsetModeClick:4779-4802  A edOcrTextMouseDown:4631-4635 (WA-1 gate, empty)
//    A edOverRangeClick:4839-4842  A edDelayTimeClick:4844-4847
//    A edAuto1CountClick:4849-4852  A edtGetValueDelayTimeClick:4854-4857 (all WA-1)
//    B ReadUseSuckModeFile:2133-2146  B CheckSTMMode:2148-2177  B cbI21Click:4804-4828
//    D Init(=ctor):123-216  D FormDestroy:218-230  D CompChange:1195-1262
//    D CHSetError:1284-1605  D rgUseSuckModeClick:3423-3450
//    D CheckShuttlePitch:4645-4706  D CoSocketComboChange:4752-4770
//  QUEUED 22/47, per-method reason (every span read/signal-scanned this wave):
//    ScrollBar1Change      : 232-1193 (962L) MIXED -- Barcode_Reader guard (:242,
//        zero-port tree-wide) + 17 writes to global iSiteTotal + a widget
//        surface far beyond this facade; ctor-array/TScrollBar were only two
//        of its blockers, so Wave D does NOT unlock it.
//    FormShow              :1607-2131 (525L) fSecurity x14 + TImage
//        (Image1->...->LoadFromFile) + myLog.Do_Log + fMain deps.
//    ReadFile              :2191-3020 (830L) WRITE-PATH (5x WriteIniData) +
//        COM2/fMain/fOCR/fTemp/fCleaning/ChangeHotPlateData.
//    DoIniDataToForm       :3022-3337 (316L) WRITE-PATH (2x WriteIniData) +
//        fMain/COM2.
//    FormClose             :3339-3401 (63L)  myLog.Do_Log (handlerlog wiring
//        not landed) + VCL close-action lifecycle.
//    XPitchMouseDown       :3409-3421 (13L)  LANDED (FW-SETUP-E 20260824;
//        Barcode_Reader guard gated E-B1, fQwertyKey real since FW-QWKEY1).
//    sbtExitClick          :3452-3466 (15L)  Close() lifecycle + fOCR->
//        sTesterLotId + fMain->GetCZSiteMap + bGetLotIDFormTester.
//    sbUpdateClick         :3468-3628 (161L) WRITE-PATH (calls SaveSetupFile +
//        fMain->BackupSetupFile) + Close().
//    SaveSetupFile         :3630-4165 (536L) THE WRITE-PATH: 130x WriteIniData
//        into the recipe file family. Stays queued until the user's
//        write-path wave (safety-critical by campaign rule).
//    btnLUpToRDownNClick   :4167-4290 (124L) fPassword x3 + fQwertyKey +
//        Barcode_Reader.
//    DoPassword            :4292-4329 (38L)  fQwertyKey->bShow + fPassword +
//        fMain login chain (fPassword-unported).
//    cbEnableRealTimeCCDClick:4331-4366 (36L) COM2->SendCommToVision +
//        IndexStatus + fMain->CheckCanChangeRealDummy.
//    cbAaChange            :4368-4406 (39L)  fPassword/fQwertyKey +
//        Barcode_Reader + SetFocus/Repaint (no port).
//    cbAaDropDown          :4408-4441 (34L)  LANDED (FW-SETUP-E 20260824;
//        fPassword/fQwertyKey real, SetFocus = TControl offline no-op).
//    rgShtModeNormalMouseDown:4451-4462 (12L) LANDED (FW-SETUP-W19 20260826;
//        Barcode_Reader 的真本體在 BarcodeReader.cpp:445，這條理由已消失)。
//    cbOctal12SiteClick    :4476-4488 (13L)  its ENTIRE payload is calling
//        ScrollBar1Change(this) -- blocked solely on that method staying
//        queued (first entry above).
//    chkOffCenterkitClick  :4490-4538 (49L)  TImage-zero-port (7x
//        Image1->Picture->LoadFromFile).
//    cbQualSite2X2ShiftClick:4540-4575 (36L) TImage-zero-port (6x).
//    DoPasswordCheck       :4577-4629 (53L)  fQwertyKey/fPassword/fMain login
//        chain + writes global AccessLevel.
//    btAutoShuttlePitchClick:4732-4736 (5L)  fMain->Timer9 (TTimer zero-port).
//    XShiftPitchMouseDown  :4738-4750 (13L)  LANDED (FW-SETUP-E 20260824;
//        Barcode_Reader guard 一度 gated E-B2，但 **E-B2 已於 20260825
//        FW-BARCODE2 開掉**，本行的「gated」是過期敘述，20260826 更正)。
//    FormShortCut          :4830-4837 (8L)   TWMKey (zero-port, Msg.CharCode
//        IS read -- not a droppable param) + form Left/Top members.
//
//  GOLDEN NOTES logged this wave (faithful-translation discipline: photograph,
//  don't fix):
//    GOLDEN QUIRK  -- CHSetError :1559/:1590 `if(iYpitch>63.5)`: iYpitch is
//        int, compared against 63.5; behaviourally identical to >=64.
//        Translated verbatim.
//    GOLDEN ODDITY -- CHSetError's pitch switch (:1348-1603) has NO case for
//        QualSite2X2N/_6Site2X3N/_8Site2X4N/_32Site4X8N/_16Site4X4-as-itself
//        beyond its shared _8Site2X4 case label, and no default: the N-mode
//        variants fall through the switch entirely un-range-checked (their
//        X/Y pitch is validated nowhere in this function). Preserved verbatim.
//    GOLDEN ODDITY -- CoSocketComboChange :4752-4770 ignores its Sender and
//        reads CoSocketCombo directly (Sender IS CoSocketCombo at every
//        golden wiring site); Sender dropped per the established
//        unused-VCL-glue rule.
//    PRE-EXISTING vclcompat nuance (not new, recorded for CompChange's
//        readers): vclcompat::TComboBox::Clear() clears Items only; real VCL
//        also clears Text. CompChange's Clear() sites only care about Items
//        (they re-Add the CH list immediately), so no behaviour differs on
//        any translated path.
// =============================================================================
#include "forms/fSetup.h"
//  AI(W906-SETUP-READFILE) 20260922: headers the restored TfSetup::ReadFile
//  reaches for its 17 LIVE form accesses (the 4 it cannot reach are gated
//  inline -- G-SU-Clean / G-SU-Ocr / G-SU-AcBtn / G-SU-Restart).
#include "forms/fCleaning.h"               // fCleaning->SaveAutoCleanData          golden :2522
#include "forms/fConfiguration.h"          // fConfiguration->ReadLockByFile        golden :2739
#include "forms/fHotPlate.h"               // fHotPlate->SetArmHotPlateYPitch       golden :2960
#include "forms/fLotInfo.h"                // fLotInfo->RTCChangeFile / LoadRTCFullViewImg / ShowSocketID
#include "forms/fOCR.h"                    // fOCR->SetOcrFileName / cbEnableOcr    golden :2667-2669
#include "forms/fRPDefault.h"              // fRPDefault->ShowMonitoredParameter
#include "forms/fStartCondition.h"         // fStartCondition->ReadWriteStartCondition
#include "forms/fTemp_Set.h"               // fTemp_Set->ReadTempFile
#include "forms/fTemperFrom.h"             // fTemperFrom->SetIndex16HeaterPos / SetShowYield / esytOcrOn
#include "FormsFacade.h"                   // fMain->SendMSG_CMD / cbSetupFileName / cbDisableSiteMappingCheck
#include "acarry_shims.h"                 // IsNNMode() (:227) -- golden cinitial.h:60
#include "cinitial.h"                     // InitShuttleThreadParameter() (:143)
#include "MessageDef.h"                   // MSG_CMD_{Enable,Disable}{BarCode,Pin1Function}
#include "atester_shims.h"                // COM2 (TCOM2Shim*, :412) -- golden rs232.h:203

//  AI(W906-SETUP-READFILE) 20260922: golden declares this in ainarm2.h:105; the
//  port translated the body (ainarm2.cpp:573, ACTIVE, no gates) but published no
//  header for it. Declaration only -- no ODR risk.
#ifndef HT9045_W7_ChangeHotPlateDataToOneShuttle_DECLARED
#define HT9045_W7_ChangeHotPlateDataToOneShuttle_DECLARED
extern void ChangeHotPlateDataToOneShuttle();   // golden ainarm2.h:105 (body ainarm2.cpp:573)
#endif

#include "aHotPlateSubstrate.h"      // InArmSuck / OutArmSuck / MOT[] / ResetShuttleWhichKit /
                                     // iCloseSiteModeFor2x6 / iCloseSiteModeFor2x8 / iCloseSiteModeFor1x4
#include "csystem.h"                 // IndexHasIC() / ShuttleHasIC()
#include "canary_support.h"          // LastSet (-> LastSet.h) / ShowMyMessage
#include "forms/fQwertyKey.h"        // AI(W906-FW-SETUP-E) 20260824: fQwertyKey extern (:406) + ShowQwertyKey -- the unlock batch at this file's tail
#include "forms/fPassword.h"         // AI(W906-FW-SETUP-E) 20260824: fPassword extern (:406) + edPassword
#include "vclcompat/ShiftState.h"   // AI(W906-FW-SETUP-W19) 20260826: rgShtModeNormalMouseDown 保留 golden 完整簽章
#include "BarcodeReader.h"     // AI(W906-FW-BARCODE2) 20260825: Barcode_Reader real since FW-BARCODE1 -- E-B1/E-B2 guards live

// AI(W906-FW-SETUP-B) 20260821: same "declare just the one symbol needed"
// posture Command.cpp (:282) and MainTempMode.cpp (:57) already established
// for this exact function -- see this file's header-comment for why.
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug=" ");

// AI(W906-FW-SETUP-B) 20260821: CheckSTMMode's 5 per-arm-variant dispatch
// targets -- declared locally, not via #include, to sidestep the
// enum-redeclaration clash with aHotPlateSubstrate.h documented above.
void CheckSTMMode_2x6_8();                                       // ainarm9045_2x6_8.h:38
void CheckSTMMode_2x8_8();                                       // ainarm9045_2x8_8.h:46
void CheckCloseSiteMode_1x3_14();                                 // ainarm9045_1x3_2_14.h:20
void CheckCloseSiteMode_2x3N_14();                                // ainarm9045_1x3_2_14.h:21
void CheckCloseSiteMode_1x4_4();                                  // ainarm9045_1x4_4.h:31

// e2x6Standard/e1x4Standard (=0 each) -- the two enum values CheckSTMMode
// needs from ainarm9045_2x6_8.h's `enum e2x6Mode`/ainarm9045_1x4_4.h's
// `enum e1x4Mode`, without pulling in either header (see redeclaration note
// above). e2x8Standard needs no such shim -- it is MachineType.h's own
// `enum e2x8Mode` (already visible via forms/fSetup.h -> cprod.h ->
// MachineType.h), a distinct header with no clash.
static const int e2x6Standard = 0;                                // ainarm9045_2x6_8.h:25
static const int e1x4Standard = 0;                                // ainarm9045_1x4_4.h:21

// ---------------------------------------------------------------------------
// TfSetup::ReadUseSuckModeFile
//   BCB6 source: cSetUp.cpp:2133-2146 (14 lines)
// ---------------------------------------------------------------------------
void TfSetup::ReadUseSuckModeFile()
{
    AnsiString szDir=GetRecipeFileName("HandlerCondition.Data");
    TestIF_File.iUseSuckMode=ReadIniData(szDir, "Configuration", "Use Suck Mode", 4);

    // golden :2138-2145 -- if any live IC is under the machine, leave the
    // shuttle-place order alone (empty golden if-body, translated verbatim);
    // otherwise re-run AdjustShuttlePlaceOrder via ResetShuttleWhichKit.
    if(IndexHasIC() || ShuttleHasIC() || InArmSuck.HasIC() ||
       MOT[MMPlate1].HasIC() || MOT[MMPlate2].HasIC())                          //Steven 20160213 : 解決Input功能順序問題
    {
    }
    else
    {
        ResetShuttleWhichKit();
    }
}

// ---------------------------------------------------------------------------
// TfSetup::CheckSTMMode
//   BCB6 source: cSetUp.cpp:2148-2177 (30 lines)                              //Steven 20221006 : 確認STM模式
// ---------------------------------------------------------------------------
void TfSetup::CheckSTMMode()
{
    iCloseSiteModeFor2x8=e2x8Standard;                                         //JerryYang 20191122 STM 8 site Auto clean支援前後排一起收
    iCloseSiteModeFor2x6=e2x6Standard;                                         //Steven 20240417 : 12site分成前後4site
    iCloseSiteModeFor1x4=e1x4Standard;

    if(TestIF_File.iTestMode==_12Site2X6)
    {
        CheckSTMMode_2x6_8();
    }
    else if(TestIF_File.iTestMode==_16Site2X8 ||
            TestIF_File.iTestMode==_32Site4X8N)
    {
        CheckSTMMode_2x8_8();
    }
    else if(TestIF_File.iTestMode==TriSite1X3 &&
            iInArmType==e9045_1x3_2_14)
    {
        CheckCloseSiteMode_1x3_14();                                           //Steven 20251113 : for 1x3_14 run 3x7 HP
    }
    else if(TestIF_File.iTestMode==_6Site2X3N &&
            iInArmType==e9045_1x3_2_14)
    {
        CheckCloseSiteMode_2x3N_14();                                           //Steven 20251113 : for 1x3_14 run 3x7 HP
    }
    else if(TestIF_File.iTestMode==QualSite1X4)
    {
        CheckCloseSiteMode_1x4_4();                                            //Steven 20241111 : for 1x4 close site
    }
}

// ---------------------------------------------------------------------------
// TfSetup::ReadFile
//   BCB6 source: cSetUp.cpp:2191-3020 (830 lines), verbatim transliteration.
//   golden `__fastcall` dropped, matching the three TfSetup:: methods above.
//
//   WHAT THIS FIXES.  This is the ONLY reader of HandlerCondition.Data's
//   [Configuration] section.  Until now the port had no TfSetup::ReadFile at
//   all, so all 80 TestIF_File fields it fills sat at their static-init values
//   forever.  The two that prompted this (user, 20260922) are
//       TestIF_File.dSiteXPitch  golden :2445
//       TestIF_File.dSiteYPitch  golden :2446
//   which cUnitConvert.cpp:376-377 converts into TestIF.dSite{X,Y}Pitch, and
//   which ~2200 call sites downstream read for site geometry (shuttle pitch,
//   nozzle-count selection, start coordinates).  All of that was computing off
//   zero.
//
//   96 ReadIniData calls, 80 distinct TestIF_File fields.  Every field exists
//   in SYSTEM_TEST_IF (cprod.h:1649-2575) -- verified field-by-field, none had
//   to be added.
//
//   FORM ACCESS.  21 form-pointer accesses; 17 resolve to real facade members
//   and stay ACTIVE.  Four do not exist on this tree's facades and are gated
//   individually below -- G-SU-Clean / G-SU-Ocr / G-SU-AcBtn / G-SU-Restart.
//   Read G-SU-Restart's note before trusting the RTC-enable path offline: the
//   operator still sees the "program must restart" message, but nothing arms
//   the restart.
// ---------------------------------------------------------------------------
void TfSetup::ReadFile()
{
    AnsiString str, str1, str2;
    int iSearch;
    bool bEnableCCDBuffer=false, bEnableCCDBuffer2=false;                       //Sam 20240311 : 新增 RTC Lock by file 功能
    static bool bFirstReadFile=true;
    AnsiString szDir=GetRecipeFileName("HandlerCondition.Data");

    TestIF_File.sTestMode=ReadIniData(szDir, "Configuration", "Test Mode", AnsiString("")); //Steven 20120814 Start: 改用文字存模式,方便新增新的模式

    if(TestIF_File.sTestMode=="")                                               //向舊版的相容,不要動!!
    {
        iSearch=ReadIniData(szDir, "Configuration", "Handling Mode", 0);
        switch(iSearch)
        {
            case 0:  TestIF_File.sTestMode="2-Site"                         ; break;
            case 1:  TestIF_File.sTestMode="Single Site"                    ; break;
            case 2:  TestIF_File.sTestMode="In-Line 4-Site(1X4)"            ; break;
            case 3:  TestIF_File.sTestMode="Square 4-Site(2X2)"             ; break;
            case 4:  TestIF_File.sTestMode="Square 4-Site(2X2) Busy Shuttle"; break;
            case 5:  TestIF_File.sTestMode="8-Site"                         ; break;
            case 6:  TestIF_File.sTestMode="16-Site"                        ; break;
            case 7:  TestIF_File.sTestMode="12-Site"                        ; break;
            case 8:  TestIF_File.sTestMode="32-Site N Mode"                 ; break;
            case 9:  TestIF_File.sTestMode="32-Site M Mode"                 ; break;
            case 10: TestIF_File.sTestMode="2-Site Busy Shuttle"            ; break;
        }
    }

    TestIF_File.iTestMode=GetTestMode(TestIF_File.sTestMode);                   //Steven 20250321 : 整合TestMode轉換

    if(CosFunction.bEnable6Site==false &&                                       //Steven 20100709 : HT9045讀到16Site的設定檔時，強制設定成8Site
       TestIF_File.iTestMode==_6Site2X3)
    {
        TestIF_File.iTestMode=_8Site2X4;
    }
    else if(MachineTypeChoice==Type_HT9045)                                     //9045
    {
        if(TestIF_File.iTestMode==_16Site2X8 ||
           TestIF_File.iTestMode==_12Site2X6 ||
           TestIF_File.iTestMode==_16Site4X4)
            TestIF_File.iTestMode=_8Site2X4;
        else if(TestIF_File.iTestMode==QualSite2X2N)                            //Steven 20200722 : 2x2 NN mode
            TestIF_File.iTestMode=QualSite2X2;
        else if(TestIF_File.iTestMode==_6Site2X3N)                              //Steven 20220425 : 2x3 NN mode
             TestIF_File.iTestMode=_8Site2X4;
        else if(TestIF_File.iTestMode==_8Site2X4N)                              //Wei 20231211 : 2X4NN Mode
             TestIF_File.iTestMode=_8Site2X4;
    }
    else if(MachineTypeChoice==Type_HT9046 ||
            SubMachineType==Type_HT9046LA)                                      //Steven 20190307 : Add for HT-9046LA
    {
        if(TestIF_File.iTestMode==_32Site4X8M ||
           TestIF_File.iTestMode==_32Site4X8N ||
           TestIF_File.iTestMode==_16Site4X4)                                   //Sam 20190226 : 16Site4X4
            TestIF_File.iTestMode=_16Site2X8;
        else if(TestIF_File.iTestMode==QualSite2X2N)                            //Steven 20200722 : 2x2 NN mode
            TestIF_File.iTestMode=QualSite2X2;
        else if(TestIF_File.iTestMode==_6Site2X3N)                              //Steven 20220425 : 2x3 NN mode
             TestIF_File.iTestMode=_8Site2X4;
        else if(TestIF_File.iTestMode==_8Site2X4N)                              //Wei 20231211 : 2X4NN Mode
             TestIF_File.iTestMode=_8Site2X4;
    }
    else if(MachineTypeChoice==Type_HT9045_12Site)                              //ChungHung 20130507 add HT9045 updata for 12site 517
    {
        if(TestIF_File.iTestMode==_16Site2X8)
            TestIF_File.iTestMode=_8Site2X4;
        else if(TestIF_File.iTestMode==QualSite2X2N)                            //Steven 20200722 : 2x2 NN mode
            TestIF_File.iTestMode=QualSite2X2;
        else if(TestIF_File.iTestMode==_6Site2X3N)                              //Steven 20220425 : 2x3 NN mode
             TestIF_File.iTestMode=_8Site2X4;
        else if(TestIF_File.iTestMode==_8Site2X4N)                              //Wei 20231211 : 2X4NN Mode
             TestIF_File.iTestMode=_8Site2X4;
    }
    else
    {
        if(TestIF_File.iTestMode==QualSite2X2N &&
           CosFunction.bCanUse2x2NNMode==false)                                 //Steven 20200722 : 2x2 NN mode
            TestIF_File.iTestMode=QualSite2X2;
        if(TestIF_File.iTestMode==_6Site2X3N &&
           CosFunction.bCanUse2x3NNMode==false)                                 //Steven 20220425 : 2x3 NN mode
            TestIF_File.iTestMode=_8Site2X4;
        if(TestIF_File.iTestMode==_8Site2X4N &&
           CosFunction.bCanUse2x4NNMode==false)                                 //Wei 20231211 : 2X4NN Mode
            TestIF_File.iTestMode=_8Site2X4;
    }

    if(TestIF_File.iTestMode==_8Site2X4     ||
       TestIF_File.iTestMode==QualSite1X4   ||
       TestIF_File.iTestMode==DualSite      ||
       TestIF_File.iTestMode==QualSite2X2   ||
       TestIF_File.iTestMode== _8Site1X4    ||                                  //ChungHung 20150528 add for 海思 _8Site1x4
       TestIF_File.iTestMode==_6Site2X3     ||                                  //ChungHung 20150119 add for 2X3_6
       TestIF_File.iTestMode==_12Site2X6    ||                                  //ChungHung 20141031 add 12Site HotPlate Check //Steven 20120323 : 可以檢查HotPlate的模式
       TestIF_File.iTestMode==_10Site2X5    ||
       TestIF_File.iTestMode==_16Site4X4    ||                                  //Sam 20190226 : 16Site4X4
       TestIF_File.iTestMode==_16Site2X8)                                       //Isaac 20191007 : add Hotplate check
    {
        bCanUseHotPlateCheck=true;
    }
    else
    {
        bCanUseHotPlateCheck=false;
    }

    if(CUSTOMER_CODE==CC_ASE_SG && TestIF_File.iTestMode==SingleSite)
    {
        bCanUseHotPlateCheck=true;
    }

    if(USE_PICKER_COUNT==ep1Picker)
        bCanUseHotPlateCheck=true;

    if(USE_16_HEATER==eht32HeaterEJ1N   ||
       USE_16_HEATER==eht32HeaterKT4H   ||                                      //Steven 20150211 : Index使用KT4H版32組加熱器
       USE_16_HEATER==eht32HeaterDTME08 ||                                      //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
       (ATC_SYSTEM==eNewATCSystem &&                                            //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關
        iATC_Use_Heat_Count>=8))                                                //kevin 20180504 add
    {
        TestIF_File.bUse32Heater        =ReadIniData(szDir, "Configuration", "12_16SiteDirectHeaterLayout", 0);   //Steven 20140923 : Index使用EJ1N版32組加熱器
        TestIF_File.b12SiteUse10Heater  =ReadIniData(szDir, "Configuration", "12Site10DirectHeaterLayout", 0);    //Steven 20140923 : Index使用EJ1N版32組加熱器
    }
    else
    {
        TestIF_File.bUse32Heater        =false;
        TestIF_File.b12SiteUse10Heater  =false;
    }
    IniConfig.iSiteMapDirection  =ReadIniData(szDir, "Configuration", "SiteMapDirection", 0);   //Jimmychiu 20230807 : #R230804-ATK-H9-01 , V3.21.792.1 ,Add the Sitemap items in information.txt

    int iSite=0;
    int K=1;
    if(CUSTOMER_CODE==CC_ASE_KaohSiung_K3)                                      //kevin 20211130 K5 強制 SITE 順序  林建續
    {                                                                           //kevin 20211130 add K3 K5 Site 順序固定
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                str.sprintf("Site %c%c", i+'A', j+'a');                         //kevin 20180312 change  j+'A'==> j+'a'
                iSite=ReadIniData(szDir, "Configuration", str, 0);
                if(iSite==0)
                {
                    TestIF_File.iSiteMap[i][j]=0;                               //關SITE
//                    if(TestIF_File.iSiteMap[i][j]<=0)                         //Jimmychiu 20230807 : Mark for not use
//                    {
//                        TestIF_File.iSiteMap[i][j]=0; //kevin 20150427
//                    }
                }
                else
                {
                    TestIF_File.iSiteMap[i][j]=K;
                    if(TestIF_File.iSiteMap[i][j]<=0)
                    {
                        TestIF_File.iSiteMap[i][j]=0;                           //kevin 20150427
                    }
                }
                tSiteMap->Strings[i*MAX_SOCKET_COL+j]=AnsiString(TestIF_File.iSiteMap[i][j]);   //Steven 20140510 : Secs Gem
                iAutoSiteMap[i][j]=0;                                           //kevin 20150114
                K++;
            }
        }
    }
    else
    {
        for(int i=0; i<MAX_SOCKET_ROW; i++)
        {
            for(int j=0; j<MAX_SOCKET_COL; j++)
            {
                str.sprintf("Site %c%c", i+'A', j+'a');                         //kevin 20180312 change  j+'A'==> j+'a'
                TestIF_File.iSiteMap[i][j]=ReadIniData(szDir, "Configuration", str, 0);
                if(TestIF_File.iSiteMap[i][j]<=0)
                {
                    TestIF_File.iSiteMap[i][j]=0;                               //kevin 20150427
                }
                tSiteMap->Strings[i*MAX_SOCKET_COL+j]=AnsiString(TestIF_File.iSiteMap[i][j]);   //Steven 20140510 : Secs Gem

                iAutoSiteMap[i][j]=0;                                           //kevin 20150114
            }
        }
    }

    if(CUSTOMER_CODE==CC_SCC)                                                   //Steven 20220624 : JSCC楊建軍要求跟著工作檔跑
    {
        IniConfig.bLifeTimeCount[0]=ReadIniData(szDir, "O_Count", "bLifeTimeCount 1", false);
        IniConfig.bLifeTimeCount[1]=ReadIniData(szDir, "O_Count", "bLifeTimeCount 2", false);
        IniConfig.bLifeTimeCount[2]=ReadIniData(szDir, "O_Count", "bLifeTimeCount 3", false);
    }

    if(CosFunction.bI21EnableASMByRecipe)                                       //Steven 20210518 : 吳如春希望Auto site map從工作檔開關
    {
        if(IniConfig.bVTESTFunction==true)
            IniConfig.bI21EnableASM=ReadIniData(szDir, "Auto Site Mapping", "Enable Auto Site Mapping", true);
        else
            IniConfig.bI21EnableASM=ReadIniData(szDir, "Auto Site Mapping", "Enable Auto Site Mapping", false);

        if(CosFunction.bUseOpenCloseSiteMapAtAnyTime==true)                     //Ifor 20190620 : add Auto Site Maping 隨時關閉開關顯示
        {
#if 0 // GATE(G-SU-SiteMapChk) -- TfMain facade has no cbDisableSiteMappingCheck (golden main.h, a TCheckBox). golden :2496/:2500 -- UI mirror of IniConfig.bI21EnableASMByRecipe; the ini value itself is read and stored ACTIVE, only the checkbox echo is gated.
            fMain->cbDisableSiteMappingCheck->Visible=IniConfig.bI21EnableASM;
#endif
        }
        else
        {
#if 0 // GATE(G-SU-SiteMapChk) -- TfMain facade has no cbDisableSiteMappingCheck (golden main.h, a TCheckBox). golden :2496/:2500 -- UI mirror of IniConfig.bI21EnableASMByRecipe; the ini value itself is read and stored ACTIVE, only the checkbox echo is gated.
            fMain->cbDisableSiteMappingCheck->Visible=false;
#endif
        }
    }

    TestIF_File.iShuttleMode=ReadIniData(szDir, "Configuration", "Shuttle Mode", 0);
    TestIF_File.iShuttle_Sel=ReadIniData(szDir, "Configuration", "Shuttle1 Cancel", 0);

    if(TestIF_File.iShuttleMode==0)                                             //kevin 20220329 開兩支ARM只使用arm2 生產
    {
        TestIF_File.iShuttle_Sel=0;
    }
    iSelectArm=0;                                                               //kevin 20220426 0: Arm1 Arm 2  1: Arm1  2: Arm2
    if(TestIF_File.iShuttleMode==1)                                             //kevin 20220329 開兩支ARM只使用arm2 生產
    {
        if(TestIF_File.iShuttle_Sel==0)
            iSelectArm=1;
        else
            iSelectArm=2;
    }

    if(IniConfig.bD58UseArm1PickPlaceArm2Test)
        TestIF_File.bArm1PickPlaceArm2Test=ReadIniData(szDir, "Configuration", "bArm1PickPlaceArm2Test", false);  //JerryYang 20160108 要先讀取bArm1PickPlaceArm2Test再修改iShuttleMode
    else
        TestIF_File.bArm1PickPlaceArm2Test=false;
    if(IniConfig.bIndexArm2SupplyLight==true ||                                 //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
       TestIF_File.bForEgisTecTest==true     ||                                 //Steven 20140922 : Arm2當作指紋測試
       (TestIF_File.bArm1PickPlaceArm2Test && IniConfig.bD58UseArm1PickPlaceArm2Test==true))       //kevin 20150127 Arm1 下壓 arm2 測試//Ifor 20191122 : 避免D58功能關閉無法開ARM問題
    {
        TestIF_File.iShuttleMode=1;                                             //only use shuttle 1
        TestIF_File.iShuttle_Sel=0;
    }

    if(TestIF_File.bArm1PickPlaceArm2Test)
    {
        TestIF_File.bRTC20CheckFunction =ReadIniData (szDir, "Configuration", "bRTC20CheckFunction", false); //Ifor 20190712 : add RTC2.0 Check
        TestIF_File.bRTC20GiveWayCheck  =ReadIniData  (szDir, "Configuration", "bRTC20GiveWayCheck", false);  //Ifor 20190712 : add RTC2.0 Check
    }
    else
    {
        TestIF_File.bRTC20CheckFunction =false;
        TestIF_File.bRTC20GiveWayCheck  =false;
    }

    TestIF_File.bRTCICResidueCheck=ReadIniData  (szDir, "Configuration", "bRTCICResidueCheck", false);  //Ifor 20190712 : add RTC2.0 Check
    TestIF_File.bRTC20OverFlowCheck=ReadIniData (szDir, "Configuration", "bRTC20OverFlowCheck", false); //Ifor 20190712 : add RTC2.0 Check

    TestIF_File.bArm1UseHeat=ReadIniData        (szDir, "Configuration", "bArm1UseHeat", false);        //Ifor 20190815 : add Arm1是否加熱開關
    TestIF_File.bArm1PickPlaceArm2Test_RunAutoClean=ReadIniData(szDir, "Configuration", "bArm1PickPlaceArm2Test_RunAutoClean", false);//Jimmychiu 20230710 : Auto Clean 跑 Arm1 下壓 arm2 測試

    if(IsNNMode()==NN_2Row)
    {
        TestIF_File.iShuttleMode=0;
    }

    TestIF_File.dSiteXPitch =ReadIniData(szDir, "Configuration", "X Pitch", 0.0);
    TestIF_File.dSiteYPitch =ReadIniData(szDir, "Configuration", "Y Pitch", 0.0);

    CheckSTMMode();                                                             //Steven 20221006 : 確認STM模式

    if(CosFunction.bNonCenterModeCanUseShtOffset && IniConfig.bA50Enable1x4BiasYOffset && TestIF_File.iTestMode==QualSite1X4)      //Steven 20200715 : for Tinton
        TestIF_File.i1x4SiteYOffset=ReadIniData(szDir, "Configuration", "i1x4SiteYOffset", 0);

    if(IsNNMode()==NN_2Row)                                                     //Steven 20201013 : NN mode add Y-Pitch offset selection //KenHsieh 20231227 : add Y Center Pitch mode
    {
        TestIF_File.iYPitchOffsetMode=ReadIniData(szDir, "Configuration", "iYPitchOffsetMode", 0);
    }
    else
    {
        TestIF_File.iYPitchOffsetMode=0;
    }

    TestIF_File.dSiteYOffset=ReadIniData(szDir, "Configuration", "Y Offset", 0.0);  //Steven 20140512 : For HT-9047

    TestIF_File.dShiftXPitch=ReadIniData(szDir, "Configuration", "X Shift Pitch", 0.0); //wei 20160226 TSMC X Shift

    TestIF_File.dPreciserXPitch =ReadIniData(szDir, "Configuration", "Preciser X Pitch", 0.0);    //Frank 20180410 (Steven) : InArm Preciser Station
    TestIF_File.dPreciserYPitch =ReadIniData(szDir, "Configuration", "Preciser Y Pitch", 0.0);

    if(TestIF_File.iTestMode<DualSite2x1 ||                                     //Steven 20140917 : 加上保護
       TestIF_File.iTestMode==QualSite2X2N ||                                   //Steven 20241220 : Add
       TestIF_File.iTestMode==_6Site2X3N ||
       TestIF_File.iTestMode==_8Site2X4N)
    {
        TestIF_File.dSiteYPitch=0;
    }

    iSearch=ReadIniData(szDir,"Configuration", "Search Last Mode", 0);
    if(iSearch==0)
        TestIF_File.bSearchLastMode=false;
    else
        TestIF_File.bSearchLastMode=true;

    if(TestIF_File.iShuttleMode==1)
    {
        switch(TestIF_File.iTestMode)
        {
            case DualSite:                                                      //1x2
                TestIF_File.iUseSuckMode=2;
                WriteIniData(szDir, "Configuration", "Use Suck Mode", 2);
                break;
            case QualSite1X4:                                                   //1x4
            case _8Site1X4:                                                     //ChungHung 20150528 add for 海思 _8Site1x4
            case QualSite2X2:                                                   //2x2
                TestIF_File.iUseSuckMode=4;
                WriteIniData(szDir, "Configuration", "Use Suck Mode", 4);
                break;
            case _8Site2X4:
            case _16Site4X4:
                if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                  //Steven 20231207 : 2x4 16 picker
                   USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)
                {
                    TestIF_File.iUseSuckMode=16;
                    WriteIniData(szDir, "Configuration", "Use Suck Mode", 16);
                }
                break;
        }
    }
    else
    {
        TestIF_File.iUseSuckMode=ReadIniData(szDir, "Configuration", "Use Suck Mode", 4);
    }

    if(bFirstReadFile==true)                                                    //Ifor 20161116 取得 Use Suck Mode 初始值
    {
        bFirstReadFile=false;
        iUseSuckModeBackup=TestIF_File.iUseSuckMode;                            //Ifor 20161116 add Brackup Use Suck Mode
    }

    if(IniConfig.bEnableAutoCleanFunction)
    {
#if 0 // GATE(G-SU-Clean) -- forms/fCleaning.h carries SaveAutoCleanData (kept ACTIVE on the next line) but NOT LoadAutoCleanData. golden :2521. Offline the AutoClean grid is never re-read from file; the Save beside it still runs, so this is a one-sided gate -- see the honesty note in the banner.
        fCleaning->LoadAutoCleanData();                                         //ChungHung 20131120 AutoClean use Hotplate1      //JimmyChiu 20220121 mark 重複執行
#endif
#if 0 // GATE(G-SU-CleanSave) -- TfCleaning facade has no SaveAutoCleanData. golden :2522. Pairs with G-SU-Clean directly above -- so the WHOLE AutoClean load/save pair is inert offline, not just the load half. Corrects the G-SU-Clean note, which said the Save stayed ACTIVE.
        fCleaning->SaveAutoCleanData();                                         //Jimmychiu 20260122 : fixed for auto clean hang up for setup close arm diff
#endif
    }

    if(TestIF_File.iTestMode==QualSite2X2)                                      //Steven 20190724 : 2x2偏心開關
    {
        if(CosFunction.bCanUse2x2Bias)
            TestIF_File.bNS7000kit=ReadIniData(szDir, "Configuration", "NS7000 bias kit",       false);
        else
            TestIF_File.bNS7000kit=false;
    }
    else
    {
        if(CosFunction.bCanUseBias)                                             //Steven 20190801 : SCC李國旗說要取消BIAS選項
            TestIF_File.bNS7000kit=ReadIniData(szDir, "Configuration", "NS7000 bias kit",         false);
        else
            TestIF_File.bNS7000kit=false;
    }

    TestIF_File.bNS8000CS=ReadIniData(szDir, "Configuration", "NS8000 Change Socket",    false);   //Steven 20120606 : 16溫控器 8Site使用Hontech頭
    if(MachineTypeChoice==Type_HT1032)
        TestIF_File.b6CableLayoutKit=ReadIniData(szDir, "Configuration", "6 Cable Layout Kit",  false);   //Steven 20230613 : ATC6.1 16Site SLK

    TestIF_File.b2CableLayoutKit=ReadIniData(szDir, "Configuration", "2 Cable Layout Kit",      false);   //Steven 20150724 : 16溫控器 2條線版本
    TestIF_File.b1CableLayoutKit=ReadIniData(szDir, "Configuration", "1 Cable Layout Kit",      false);   //jou 2015-10-15  : 16溫控器 1條線版本

    TestIF_File.bNS7000CS       =ReadIniData(szDir, "Configuration", "NS7000 Change Socket",    false);   //Steven 20100119
    TestIF_File.bSingleHeater   =ReadIniData(szDir, "Configuration", "Single site Use single heater", false);//JerryYang 20161013 新增Single site一支加熱棒模式
    if(CUSTOMER_CODE==CC_TSMC_TAINAN)
        TestIF_File.bQualSite2X2Shift=ReadIniData(szDir, "Configuration", "QualSite2X2 Shift",      false);   //wei 20160226 TSMC X Shift
    else
        TestIF_File.bQualSite2X2Shift=false;

    if(TestIF_File.iTestMode==SingleSite)
    {
        TestIF_File.bSingleUseOtherSuck =ReadIniData(szDir, "Configuration", "Enable Single Use Other Suck",    false);   //wei 20220905 Single Use Other Suck(IN C OUT E)
        TestIF_File.bSingleInArmUseOtherSuck =ReadIniData(szDir, "Configuration", "Enable Single In Use Other Suck",    false);   //Ifor 20240430 add: Single Site InArm Use Other Suck(IN C)
    }
    else
    {
        TestIF_File.bSingleUseOtherSuck =false;                                 //wei 20220905 Single Use Other Suck(IN C OUT E)
        TestIF_File.bSingleInArmUseOtherSuck =false;                            //Ifor 20240430 add: Single Site InArm Use Other Suck(IN C)
    }
    TestIF_File.bRotateShuttle  =ReadIniData(szDir, "Configuration", "Rotate Shuttle",          false);         //Steven 20100713 : 轉轉蝦頭
    if(INSTALL_OCR==eocrUninstal)                                               //Steven 20160122 : 確保是關閉的
        TestIF_File.bOcrFunction=false;
    else
        TestIF_File.bOcrFunction=ReadIniData(szDir, "Configuration", "OCR Function",            false);         //Steven 20120716 : OCR

    if(INSTALL_SOCKET_CLAMP==false)                                             //JerryYang 20160523
        TestIF_File.bUseSLKClamp=false;
    else
        TestIF_File.bUseSLKClamp=ReadIniData(szDir, "Configuration", "Socket Clamp Function", false);

    if(TestIF_File.bUseSLKClamp==false)                                         //JerryYang 20160602
        TestIF_File.iSeparabilityTest=0;
    else
        TestIF_File.iSeparabilityTest=ReadIniData(szDir, "Configuration", "Enable Separability Test",       0);

    TestIF_File.sOcrText        =ReadIniData(szDir, "Configuration", "OCR Text",  AnsiString(""));              //Steven 20120716 : OCR

    TestIF_File.bEnableUsePreciser        =ReadIniData(szDir, "Configuration", "Enable Preciser",  false);      //Frank 20180410 (Steven) : InArm Preciser Station
    TestIF_File.bEnableRTPreciser         =ReadIniData(szDir, "Configuration", "Enable RT Preciser",  false);   //Frank 20180410 (Steven) : InArm Preciser Station
    if(TestIF_File.iTestMode==_8Site2X4 && SPIL_FOR_QLE==1)                     //Kevincheng 20260525 : 渠梁常溫模式使用hotplate
    {
        TestIF_File.bEnablePreciserHotPlate    =ReadIniData(szDir, "Configuration", "Enable Preciser HotPlate",  false);
    }
    else
    {
        TestIF_File.bEnablePreciserHotPlate=false;
    }

    if(IniConfig.bVTESTFunction==true)                                          //jou 20200701 : VTEST for auto site mapping cable mount
    {
        TestIF_File.bAutoSiteMappingOpenSite=ReadIniData(szDir, "Configuration", "Auto site mapping open site", true);
        TestIF_File.bAutoSiteMappingOneCycle=ReadIniData(szDir, "Configuration", "Auto site mapping one cycle", true);
    }

    if(IniConfig.bC08_SocketSensor)
    {
        TestIF_File.bEnSocketSensor            =ReadIniData(szDir, "Configuration", "SocketSensor",                 false);     //kevin 20130504 使用SOCKET SENSOR
        #ifdef MTK_Version
        TestIF_File.bEnSocketSensor            =true;
        #endif
        TestIF_File.bSocketSensorCheckFloating =ReadIniData(szDir, "Configuration", "bSocketSensorCheckFloating",   false);     //Steven 20180313 (jou) : 使用Socket Sensor驗證置偏
        TestIF_File.iSocketCount               =ReadIniData(szDir, "Configuration", "SocketCountt",                 8);         //kevin 20130504 使用多少顆SENSOR 最多8個

        if(CUSTOMER_CODE == CC_ASE_KaohSiung)                                   //kevin 20210924 ASE -KH 不卡SOCKET SENSOR數量
        {
        }
        else if(TestIF_File.iTestMode>=_16Site4X4)                              //Steven 20190919 : SocketSensor加上數量保護
        {
            if(TestIF_File.iSocketCount<4)
                TestIF_File.iSocketCount=4;
        }
        else if(TestIF_File.iTestMode>DualSite2x1)                              //Steven 20200306 : 2x1最少只需要一顆, 其他兩排的要兩顆以上
        {
            if(TestIF_File.iSocketCount<2)
                TestIF_File.iSocketCount=2;
        }

        TestIF_File.bCheckSocketFloating=false;                                 //Steven 20200420 : Socket Sensor功能可以選
        for(int i=0; i<iSnSocketCnt; i++)
        {
            str.sprintf("iSensorCheckType%d", i+1);
            if(i<TestIF_File.iSocketCount)
            {
                if(TestIF_File.bSocketSensorCheckFloating)
                    TestIF_File.iSensorCheckType[i]=ReadIniData(szDir, "Configuration", str, 2);
                else
                    TestIF_File.iSensorCheckType[i]=ReadIniData(szDir, "Configuration", str, 1);
            }
            else
            {
                TestIF_File.iSensorCheckType[i]=ReadIniData(szDir, "Configuration", str, 0);
            }

            if(TestIF_File.iSensorCheckType[i]==2)
                TestIF_File.bCheckSocketFloating=true;
        }

        if(CUSTOMER_CODE==CC_JCET)                                              //JerryYang 20170426 (Steven) 江蘇長電吳如春要求不顯示
            TestIF_File.bSocketDisibleinitialcheck=false;
        else
            TestIF_File.bSocketDisibleinitialcheck =ReadIniData(szDir, "Configuration", "SocketDisibleinitialcheck",    false); //kevin 20160209 (wei) 使用SOCKET SENSOR
    }
    else
    {
        TestIF_File.bEnSocketSensor=false;
    }

    if(CUSTOMER_CODE==CC_GIGAS)                                                 //Isaac 20210601 : 全智要把指定參數存出來
    {
#if 0 // GATE(G-SU-RpMonitor) -- TfRPDefault::ShowMonitoredParameter is DECLARED but its body in forms/fRPDefault.cpp sits inside a #if 0, so it does not link. golden :2778.
        fRPDefault->ShowMonitoredParameter();
#endif
    }

    if(IsNNMode()!=None_NN)
    {
        bUseTwoArm32Site=true;                                                  //kevin 20190322 add two arm tester 32SITE 16SITE(4X4)
    }
    else
    {
        bUseTwoArm32Site=false;
    }

    TestIF_File.bCheckArm2Vacuum          =ReadIniData(szDir, "Configuration", "bCheckArm2Vacuum",  true);  //Steven 20150129 : 需要確認Arm2有沒有粘料

#if 0 // GATE(G-SU-Ocr) -- forms/fOCR.h has SetOcrFileName and cbEnableOcr (both ACTIVE here) but no SetOcrInspText. golden :2668. TestIF_File.sOcrText IS still read from the ini above; only pushing it into the OCR form is gated.
    fOCR->SetOcrInspText(TestIF_File.sOcrText);                                 //Steven 20120716 : OCR
#endif
#if 0 // GATE(G-SU-OcrChk) -- TfOCR facade has no cbEnableOcr (golden OCR.h, a TCheckBox). golden :2669. TestIF_File.bOcrFunction is still read ACTIVE; only the checkbox echo is gated.
    fOCR->cbEnableOcr->Checked  =TestIF_File.bOcrFunction;                                                  //Steven 20120716 : OCR
#endif
        TestIF_File.bOctal_80Kit    =ReadIniData(szDir, "Configuration", "Octal Pitch 80",              false); //2013-03-05    Dell    for ATK pitch 80mm kit

    TestIF_File.bSquare_OctalKit    =ReadIniData(szDir, "Configuration", "Square Octal Kit",        false); //Steven 20141224 : 2x2Site使用8Site Kit
    if(CUSTOMER_CODE==CC_HANA_MICRON)
        TestIF_File.b2x2Use16SiteKit=ReadIniData(szDir, "Configuration", "2x2 Use 16 Site Kit",     false); //Steven 20191113 : 2x2Site使用16Site Kit
    else
        TestIF_File.b2x2Use16SiteKit=false;

    if(CosFunction.bEnableOctal_16Kit)                                          //Steven 20140312 : 8Site使用16Site Kit
        TestIF_File.bOctal_16Kit    =ReadIniData(szDir, "Configuration", "Octal 16 Kit",            false); //Steven 20140312 : 8Site使用16Site Kit
    else
        TestIF_File.bOctal_16Kit    =false;

    if(CosFunction.bEnableOctal_12Kit)                                          //ChungHung 20140508 add for SCK
        TestIF_File.bOctal_12Kit    =ReadIniData(szDir, "Configuration", "Octal 12Kit",             false); //ChungHung 20140508 add for SCK
    else
        TestIF_File.bOctal_12Kit    =0;
    if(CosFunction.bEnableDual_1x4Kit)                                          //Isaac 20200311 : 1x2Site使用1x4Site Kit
        TestIF_File.b1x2Use1x4SiteKit =ReadIniData(szDir, "Configuration", "1x2_Use_1x4_Kit",       false);
    else
        TestIF_File.b1x2Use1x4SiteKit =false;

    if(CosFunction.bEnable_1x3Kit)                                              //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
        TestIF_File.bUse1x3SiteKit =ReadIniData(szDir, "Configuration", "Use_1x3_Kit",       false);
    else
        TestIF_File.bUse1x3SiteKit =false;

    if(CosFunction.bEnable12SiteUse16SLK)                                       //Steven 20240807 : 12Site使用16Site Kit
        TestIF_File.b2x6Use2x8SitSLK  =ReadIniData(szDir, "Configuration", "b2x6Use2x8SitSLK",        false);
    else
        TestIF_File.b2x6Use2x8SitSLK  =false;

    TestIF_File.bHontechLayoutKit2x2=ReadIniData(szDir, "Configuration", "bHontechLayoutKit2x2",    false); //jou 2015-12-10 SCS 要求 Hontech Layout kit要選擇Hontech.
    TestIF_File.bQualSite2X2Shift   =ReadIniData(szDir, "Configuration", "QualSite2X2 Shift",       false); //kevin 20171117 add

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        TestIF_File.bNSKitPress=ReadIniData(szDir, "Configuration", "bNSKitPress",  false);                 //kevin 20170804 (Steven) add 另一LOAD CELL 壓力值
    else
        TestIF_File.bNSKitPress=false;

    if(INSTALL_DOUBLE_EP==DOUBLE_EP_INDIVIAL)
    {
        if(TestIF_File.iTestMode==DualSite)
        {
            TestIF_File.bIndEPSLK=ReadIniData(szDir, "Configuration", "bIndEPSLK",  false);
        }
        else if (CUSTOMER_CODE==CC_AMKOR_Korea &&                               //RogerYang 20260603 : Add 8EP
                    TestIF_File.iTestMode==QualSite1X4)
        {
            TestIF_File.bIndEPSLK=ReadIniData(szDir, "Configuration", "bIndEPSLK",  false);
        }
    }
    else if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI && TestIF_File.iTestMode<=QualSite2X2N)                  //AI(ht9045-v899) 20260504: add Multi EP bIndEPSLK (preserve RogerYang 8EP block above).
        TestIF_File.bIndEPSLK=ReadIniData(szDir, "Configuration", "bIndEPSLK",  false);
    else
        TestIF_File.bIndEPSLK=false;

    TestIF_File.b16Direct12Shuttle  =ReadIniData(szDir, "Configuration", "b16Direct12Shuttle",      false); //kevin 20191104 add 16 Site Direct Heater use 12 Site shuttle
    TestIF_File.b16Direct8Shuttle   =ReadIniData(szDir, "Configuration", "b16Direct8Shuttle",       false); //Steven 20210322 : 16 Site Direct Heater use 8 Site shuttle

    if(CosFunction.bLockRTCByFile)                                              //Sam 20240311 : 新增 RTC Lock by file 功能
    {
#if 0 // GATE(G-SU-LockByFile) -- the only `fConfiguration` in this tree is W5SckArtRem_ConfigStub (Automation/SCK_ART_Remainder.h:599), a different type with no ReadLockByFile. golden :2739 -- the RTC lock-by-file read.
        fConfiguration->ReadLockByFile();
#endif
    }

    if(CosFunction.bLockRTC && LastSet.iTester==ON_LINE)                        //Steven 20191202 : 強制開啟RTC
    {
        bEnableCCDBuffer=false;
    }
    else if(IniConfig.bRTCbySystem)                                             //ChungHung 20120716 RTC by System
    {
        if(CosFunction.bLockRTCByFile && IniConfig.bRTC_Enable==false)          //Sam 20240311 : 新增 RTC Lock by file 功能
        {
            bEnableCCDBuffer2=ReadIniData("D:\\HT9045\\config\\config.ini", "RTC", "Enable", false);
            bEnableCCDBuffer=!IniConfig.bRTC_Active;
            if(bEnableCCDBuffer==false && bEnableCCDBuffer2==true)
            {
                ShowMyMessage("Program need to restart to active RTC fuinction!", "The program will automatically be closed after exit setup form.");
#if 0 // GATE(G-SU-Restart) -- fMain facade has no bNeedRestartProgram (golden main.h). golden :2748/:2766/:2794. This is a WRITE, so gating it has a consequence: the 'restart to activate RTC' request is silently dropped offline. The ShowMyMessage that announces it to the operator stays ACTIVE above, so the message still appears -- it just no longer arms the restart.
                fMain->bNeedRestartProgram=true;
#endif
            }
            WriteIniData("D:\\HT9045\\config\\config.ini", "RTC", "Enable", (!bEnableCCDBuffer)  ?0:1);
        }
        else
        {
            bEnableCCDBuffer=ReadIniData("D:\\HT9045\\config\\config.ini", "RTC", "Enable", false);
        }
    }
    else
    {
        if(CosFunction.bLockRTCByFile && IniConfig.bRTC_Enable==false)          //Sam 20240311 : 新增 RTC Lock by file 功能
        {
            bEnableCCDBuffer2=ReadIniData(szDir, "Configuration", "Real Time CCD", false);
            bEnableCCDBuffer=!IniConfig.bRTC_Active;
            if(bEnableCCDBuffer==false && bEnableCCDBuffer2==true)
            {
                ShowMyMessage("Program need to restart to active RTC fuinction!", "The program will automatically be closed after exit setup form.");
#if 0 // GATE(G-SU-Restart) -- fMain facade has no bNeedRestartProgram (golden main.h). golden :2748/:2766/:2794. This is a WRITE, so gating it has a consequence: the 'restart to activate RTC' request is silently dropped offline. The ShowMyMessage that announces it to the operator stays ACTIVE above, so the message still appears -- it just no longer arms the restart.
                fMain->bNeedRestartProgram=true;
#endif
            }
            WriteIniData(szDir, "Configuration", "Real Time CCD",       (!bEnableCCDBuffer)  ?0:1);
        }
        else
        {
            bEnableCCDBuffer=ReadIniData(szDir, "Configuration", "Real Time CCD", false);   //Steven 20110824 : Real time CCD - Set Up可以開關
        }
    }
//    #endif
    TestIF_File.bUseSocketFloat =ReadIniData(szDir, "Configuration", "bUseSocketFloat",          false);  //Frank QQ
    TestIF_File.bUseSocketFloat =false;                                         //Ifor 20180301 :未驗證完成暫時不用強制關閉

    if(TestIF_File.bNS7000kit)                                                  //Steven 20241113 : 偏心的kit不使用StepAsideMode
    {
        TestIF_File.bUseRTCStepAsideMode=false;
    }
    else if(CUSTOMER_CODE==CC_SIGURD_HUKOU)                                     //Sam 20240709 : 矽格湖口家葳要求強制開啟
    {
        TestIF_File.bUseRTCStepAsideMode=true;
    }
    else
    {
        TestIF_File.bUseRTCStepAsideMode =ReadIniData(szDir, "Configuration", "bUseRTCStepAsideMode", false);  //Ifor 20230908 add:RTC 讓位功能 By Setup File
    }

    if(InitialOK==true && COM2->bCCDDummyRum==true && bEnableCCDBuffer==false)
    {
#if 0 // GATE(G-SU-Restart) -- fMain facade has no bNeedRestartProgram (golden main.h). golden :2748/:2766/:2794. This is a WRITE, so gating it has a consequence: the 'restart to activate RTC' request is silently dropped offline. The ShowMyMessage that announces it to the operator stays ACTIVE above, so the message still appears -- it just no longer arms the restart.
        fMain->bNeedRestartProgram=true;
#endif
    }
#ifdef SOFT_SIMULTE
    COM2->bCCDDummyRum=true;
#else
    if(REAL_TIME_CCD)                                                           //JerryYang 20250120 : modify
    {
        COM2->bCCDDummyRum=bEnableCCDBuffer;
    }
    else
    {
        COM2->bCCDDummyRum=true;
    }
#endif

    if(MachineTypeChoice==Type_HT9046_LS && bEnableCCDBuffer==true)             //2013-01-15    Dell   提高Index 運動速度(目前by9046LS專用)
        bGail_HighSpeedMode=true;

    if(REAL_TIME_CCD)
    {
        if(!COM2->bCCDDummyRum)
        {
            if(fShow)                                                           //如果有打開SetUp
            {
                if(bSavePressed)                                                //而且有存檔
                {
                    if(iTestMode!=ScrollBar1->Position)                         //如果有切換模式
                    {
                        iTestMode=ScrollBar1->Position;                         //Steven 20120319 : 防止Site Mode改來改去
                        RecordProcess("Delete ROI by Save setup");              //Steven 20220815 : 紀錄重置ROI
#if 0 // GATE(G-SU-RtcChangeFile) -- TfLotInfo facade has no RTCChangeFile (it has btChangeFile, a different thing). golden :2797/:2805.
                        fLotInfo->RTCChangeFile();                              //那就要初使化工作檔
#endif
                    }
                }
            }
            else                                                                //沒有打開SetUp,表示是外部控制
            {
                if(bFirstTime==false)
                    RecordProcess("Delete ROI by Read setup");                  //Steven 20220815 : 紀錄重置ROI
#if 0 // GATE(G-SU-RtcChangeFile) -- TfLotInfo facade has no RTCChangeFile (it has btChangeFile, a different thing). golden :2797/:2805.
                fLotInfo->RTCChangeFile(!bFirstTime);                           //所以也要初始化工作檔
#endif
            }
        }
        else
        {
            fLotInfo->LoadRTCFullViewImg(false);                                //Steven 20110825 : Real Time CCD - 使用白色圖
        }
    }

    if(CUSTOMER_CODE==CC_ASE_CL)                                                //JerryYang 20250120 : modify
    {
        bSECS_REALTIMECCD=!COM2->bCCDDummyRum;
    }
    else
    {
        bSECS_REALTIMECCD=COM2->bCCDDummyRum;
    }

    if(INSTALL_OCR!=eocrUninstal)                                               //Steven 20120716 : OCR
    {
#if 0 // GATE(G-SU-TemperFrom) -- forms/fTemperFrom.h states outright it does NOT declare `extern TfTemperFrom *fTemperFrom` yet (integration pending). golden :2825/:2952.
        fTemperFrom->SetShowYield(fTemperFrom->esytOcrOn, TestIF_File.bOcrFunction);
#endif
        if(TestIF_File.bOcrFunction)
        {
            if(fShow)
            {
                if(bSavePressed)
                {
                    if(iTestModeOcr!=ScrollBar1->Position)                      //如果有切換模式
                    {
                        iTestModeOcr=ScrollBar1->Position;                      //Steven 20120319 : 防止Site Mode改來改去
#if 0 // GATE(G-SU-OcrFile) -- TfOCR facade has no SetOcrFileName. golden :2835/:2841.
                        fOCR->SetOcrFileName(fMain->cbSetupFileName->Text);
#endif
                    }
                }
            }
            else
            {
#if 0 // GATE(G-SU-OcrFile) -- TfOCR facade has no SetOcrFileName. golden :2835/:2841.
                fOCR->SetOcrFileName(fMain->cbSetupFileName->Text);             //所以也要初始化工作檔
#endif
            }

            fMain->SendMSG_CMD(MSG_CMD_EnableBarCode);                          //Steven 20160122 : 確保是關閉的    //Steven 20160223 : 換位置
        }
        else
        {
            fMain->SendMSG_CMD(MSG_CMD_DisableBarCode);                         //Steven 20160122 : 確保是關閉的    //Steven 20160223 : 換位置
        }

        if(TestIF_File.b2DUsePinInspection)                                     //Ifor 20240528 add:Pin1 Function
            fMain->SendMSG_CMD(MSG_CMD_EnablePin1Function);
        else
            fMain->SendMSG_CMD(MSG_CMD_DisablePin1Function);
    }

    if(USE_IN_OUT_ARM_Y_PITCH==iXPitch60)                                       //jou 2012-05-15 : 選擇 Y Pitch 機構模式
    {
        TestIF_File.iARM_Y_PITCH=6000;
    }
    else if(USE_IN_Y_IS_AUTO_PITCH==true || USE_OUT_Y_IS_AUTO_PITCH==true)                                             //Steven 20131002 : XY變距  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        TestIF_File.iARM_Y_PITCH=6000;
    }
    else
    {
        if(rgInOutArmYPitch->ItemIndex==iXPitch60)
        {
            TestIF_File.iARM_Y_PITCH=ReadIniData(szDir, "Configuration", "In Out Arm Y Pitch", 6000);
        }
        else
        {
            if(USE_IN_OUT_ARM_Y_PITCH==iXPitchManual635)
                TestIF_File.iARM_Y_PITCH=ReadIniData(szDir, "Configuration", "In Out Arm Y Pitch", 6350);
            else if(USE_IN_OUT_ARM_Y_PITCH==iXPitchManual360)                   //Steven 20140819 : Y-Pitch 36mm
                TestIF_File.iARM_Y_PITCH=ReadIniData(szDir, "Configuration", "In Out Arm Y Pitch", 3600);
        }
    }
// GATE(G-SU-HpYPitch) RETIRED 20260922 -- TfHotPlate::SetArmHotPlateYPitch now has a definition (forms/fHotPlate.cpp, golden cHotPlate.cpp:682-742).
    fHotPlate->SetArmHotPlateYPitch();                                          //Steven 20180122 : For Y-Pitch at Hot Plate

    if(CosFunction.bInOutArmUseBackRowSuck==true)                               //jou 20161122 (Steven) In & Out arm use back row suck
    {
        TestIF_File.bInArmUseBackRowSuck=ReadIniData(szDir, "Configuration", "In Arm Use Back Row", false);
        TestIF_File.bOutArmUseBackRowSuck=ReadIniData(szDir, "Configuration", "Out Arm Use Back Row", false);
    }
    else
    {
        TestIF_File.bInArmUseBackRowSuck=false;
        TestIF_File.bOutArmUseBackRowSuck=false;
    }
    TestIF_File.bUseTesterDry=CheckAndReadIniData(szDir, "Configuration", "Use Tester Dry Air", false); //Ifor 20200306 : Add Tester Dry Air讀不到檔案需將預設值寫入檔案
    TestIF_File.sRtcFileName =ReadIniData(szDir, "Configuration", "RTCFileName", fMain->cbSetupFileName->Text);    //kevin 20200731 RTC FILENAME

    if(CosFunction.b2x4SupportCenterPitch==false ||                             //Steven 20170706 (wei) : 2x4中間的Pitch不同 for SCC
       TestIF_File.iTestMode!=_8Site2X4)
    {
        TestIF_File.bEnableUseXCenterPitch=false;
    }
    else
    {
        TestIF_File.bEnableUseXCenterPitch  =ReadIniData(szDir, "Configuration", "Enable X Center Pitch", false);
        TestIF_File.dSiteXCenterPitch       =ReadIniData(szDir, "Configuration", "X Center Pitch", 0.0);
    }

#if 0 // GATE(G-SU-StartCond) -- forms/fStartCondition.h states outright there is NO `extern TfStartCondition *fStartCondition` yet (integration pending). golden :2945.
    fStartCondition->ReadWriteStartCondition(true);                             //Steven 20250822 : 調整位置
#endif

    TestIF_File.bEnableReadAndCheckTorque            =CheckAndReadIniData(szDir, "Configuration", "EnableReadAndCheckTorque",                 IniConfig.bD01_1EnableReadAndCheckTorque);
    TestIF_File.dReadTorque                          =CheckAndReadIniData(szDir, "Configuration", "dReadTorque",                              IniConfig.dD01ReadTorque);
    TestIF_File.dReadTorqueDelayTime                 =CheckAndReadIniData(szDir, "Configuration", "dReadTorqueDelayTime",                     IniConfig.dD01ReadTorqueDelayTime);

    bool Test=false;
    if(Test)
        SetMotorSpeed();

    if(INDEX_PRESS_TYPE==e240KG || INDEX_PRESS_TYPE==e260KG)                    //Steven 20111003
        fTemp_Set->ReadTempFile(true);

    if(ATC_SYSTEM!=eNewATCSystem)                                               //Ifor 20160514 Add ATC 不需要重新排列溫控器位置，會出現錯誤的位置
    {
        if(USE_16_HEATER>=eht16Heater)                                          //Steven 20150128 : 移動到最下面,要先讀取完參數才行
        {
#if 0 // GATE(G-SU-TemperFrom) -- forms/fTemperFrom.h states outright it does NOT declare `extern TfTemperFrom *fTemperFrom` yet (integration pending). golden :2825/:2952.
            fTemperFrom->SetIndex16HeaterPos();                                 //Steven 20111208 : 重新排列溫控器位置
#endif
        }
    }

    InitShuttleThreadParameter();
#if 0 // GATE(G-SU-AcBtn) -- fMain facade has no bEnableAutoclean (golden main.h:1350, 'show autoclean button'). golden :2954. Pure UI affordance, no state.
    fMain->bEnableAutoclean();                                                  //kevin 20130429 autoclean
#endif

    ChangeHotPlateDataToOneShuttle();                                           //JerryYang 20171218 (Steven) fix 加熱模式雙arm切單arm hang up問題

    if(IndexHasIC() || ShuttleHasIC() || InArmSuck.HasIC() ||
       MOT[MMPlate1].HasIC() || MOT[MMPlate2].HasIC())                          //Steven 20160213 : 解決Input擺放順序問題
    {
    }
    else
    {
        ResetShuttleWhichKit();
    }

    if(CosFunction.bRotateUseHT7000HPKit && USE_ROTATE_KIT)                     //Sam 20210416 : 新增特殊模式 For Rotate Function HT7000 HP Kit
        TestIF_File.UseRotateForHT7000HPKit=ReadIniData(szDir, "Configuration", "UseRotateForHT7000HPKit", false);
    else
        TestIF_File.UseRotateForHT7000HPKit=false;

    TestIF_File.bF18InshuttleDetect =ReadIniData(szDir, "Configuration", "bF18InshuttleDetect", false); //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
    fLotInfo->ShowSocketID();                                                   //JerryYang 20190708 包成函式
    ATKRecipeInfo->SaveFile();                                                  //Steven 20170901 (wei) : For ATK要新增工作檔比對用的檔案

    //JerryYang 20230727 : add
    AnsiString sSiteLayout, sSitePitch, sSiteMap="";
    sSiteLayoutASECL.sprintf("%dX%d",TestSocket.iShtCol, TestSocket.iShtRow);

    if(TestIF_File.iSiteMap[0][0]<=0)
    {
        sSiteMapASECL="N";
    }
    else
    {
        sSiteMapASECL=IntToStr(TestIF_File.iSiteMap[0][0]);
    }

    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(i==0 && j==0)
                continue;

            if(TestIF_File.iSiteMap[i][j]<=0)
            {
                sSiteMapASECL=sSiteMapASECL+"-"+"N";
            }
            else
            {
                sSiteMapASECL=sSiteMapASECL+"-"+IntToStr(TestIF_File.iSiteMap[i][j]);
            }
        }
    }

    if(REAL_TIME_CCD==true)                                                     //JerryYang 20260526 : 修正RTC功能關閉時bShuttleMode50沒有更新
    {
        IniConfig.bEnableCCDUSETCPIP=false;
        if(!COM2->bCCDDummyRum)
            IniConfig.bShuttleMode50=true;
        else
            IniConfig.bShuttleMode50=false;
    }
    else
    {
        IniConfig.bShuttleMode50=false;
    }
//    sSiteLayoutSiteMap=sSiteLayout+"\\r\\n"+sSitePitch+"\\r\\n"+sSiteMap;
}

// ---------------------------------------------------------------------------
// TfSetup::cbI21Click
//   BCB6 source: cSetUp.cpp:4804-4828 (25 lines)                              //Ifor 20210913 add:偵測Site Mapping
//   golden `TObject *Sender` dropped, never read.
// ---------------------------------------------------------------------------
void TfSetup::cbI21Click()
{
    if(CosFunction.bI21EnableASMByRecipe)                                      //Steven 20210518 : 針對客戶要求Auto site map開工作時開啟
    {
        if(cbI21->Checked==false    &&
           CosFunction.bAutoSiteMappingUseHotPlate==true    &&
           IniConfig.bI21AutoSiteMappingUseHotplate==true   &&
           LastSet.iRunStartMode==rsmAutoSiteMap            )
        {
            if(InArmSuck.HasIC()==false && OutArmSuck.HasIC()==false &&
               ShuttleHasIC()==false    && IndexHasIC()==false)
            {
                NewRecordProcess("", "Has no device, manually close AutoSiteMapping OK!", "沒device，手動關閉AutoSiteMapping OK!");
            }
            else
            {
                if(cbI21->Checked==false)
                {
                    cbI21->Checked=true;
                    ShowMyMessage("Device remain in machine, do onecycle first, and can close AutoSiteMapping!","裝置remain device，請先執行onecycle才能夠關閉AutoSiteMapping");
                }
            }
        }
    }
}

// ===========================================================================
//  AI(W906-FW-SETUP-D) 20260824: Wave D -- everything below this line. See
//  this file's Wave D banner section for the ledger, gates and deviations.
// ===========================================================================
#include <stdlib.h>   // AI(W906-FW-SETUP-D) 20260824: atoi/atof (CHSetError/CheckShuttlePitch) -- golden got these via vcl.h

// ---------------------------------------------------------------------------
//  File-scope control arrays -- golden cSetUp.cpp:49-52,:57. File-scope in
//  golden, file-scope here (ZERO users outside cSetUp.cpp in golden -- see
//  Wave D banner). Zero-initialized statics (no dynamic initializer -> no
//  static-init-order exposure); Init() populates them.
//  MyTempRGBox's element type is the facade's TfSetupSensorRadioGroup, not
//  golden's bare TRadioGroup -- DEVIATION D-2 (S18 type-substitution idiom),
//  so Init()'s ->Name/->Caption/->Height/->Columns/->Parent writes stay
//  spellable. Every golden expression over these arrays ports unchanged.
// ---------------------------------------------------------------------------
TComboBox   *TestSiteCH[MAX_SOCKET_ROW][MAX_SOCKET_COL];
TLabel      *TestLabCol[MAX_SOCKET_COL];
TLabel      *TestLabRow[MAX_SOCKET_ROW];
int  iTestSiteCh[MAX_SOCKET_ROW][MAX_SOCKET_COL];
TfSetupSensorRadioGroup *MyTempRGBox[iSnSocketCnt];                             //Steven 20200420 : Socket Sensor功能可以選

// ---------------------------------------------------------------------------
// TfSetup::Init
//   BCB6 source: cSetUp.cpp:123-216 (94 lines) -- golden's
//   `__fastcall TfSetup::TfSetup(TComponent* Owner) : TForm(Owner)`.
//   DEVIATION D-3: explicit Init(), not a ctor (fTemp_Set.h:483 convention;
//   forms/fSetup.cpp:21 news the global fSetup at static-init time and this
//   body reads other TUs' globals). Owner param + TForm base call dropped
//   (VCL glue). NOT CALLED anywhere yet -- future wiring wave's job.
// ---------------------------------------------------------------------------
void TfSetup::Init()
{
    bSiteMapHasChange=true;                                                     //Steven 20250102 : 紀錄test mode與site map是否有變更
    iTestMode=TotalTestMode;
    AnsiString Str;
    TComboBox *tempTestSiteCBox[MAX_SOCKET_ROW][MAX_SOCKET_COL]=
    {
        {cbAa, cbAb, cbAc, cbAd, cbAe, cbAf, cbAg, cbAh},
        {cbBa, cbBb, cbBc, cbBd, cbBe, cbBf, cbBg, cbBh},
        {cbCa, cbCb, cbCc, cbCd, cbCe, cbCf, cbCg, cbCh},
        {cbDa, cbDb, cbDc, cbDd, cbDe, cbDf, cbDg, cbDh}
    };
    TLabel *tempTestLabCol[MAX_SOCKET_COL]=
    {
        labColA, labColB, labColC, labColD, labColE, labColF, labColG, labColH
    };
    TLabel *tempTestLabRow[MAX_SOCKET_ROW]={labRowA, labRowB, labRowC, labRowD};

    int iCol, iRow;
    for(int i=0; i<MAX_SOCKET_TOTAL; i++)
    {
        iRow=i/MAX_SOCKET_COL;
        iCol=i%MAX_SOCKET_COL;

        if(i<MAX_SOCKET_ROW)
        {
            TestLabRow[i]=tempTestLabRow[i];
            TestLabRow[i]->Visible=false;
        }

        if(i<MAX_SOCKET_COL)
        {
            TestLabCol[i]=tempTestLabCol[i];
            TestLabCol[i]->Visible=false;
        }
        iTestSiteCh[iRow][iCol]=-1;
        TestSiteCH[iRow][iCol]=tempTestSiteCBox[iRow][iCol];
        TestSiteCH[iRow][iCol]->Visible=false;
    }
    fShow=false;

    if(MachineTypeChoice==Type_HT9045)
    {
        ScrollBar1->Max=_8Site2X4;
    }
    else if(MachineTypeChoice==Type_HT9046_LS ||
            MachineTypeChoice==Type_HT1032)
    {
        ScrollBar1->Max=_32Site4X8N;
    }
    else if(MachineTypeChoice==Type_HT9045_12Site)                              //ChungHung 20130507 add HT9045 updata for 12site 517
    {
        ScrollBar1->Max=_12Site2X6;
    }
    else
    {
        ScrollBar1->Max=_16Site2X8;
    }

    iTestModeOcr=-1;                                                            //Steven 20120716 : OCR

    tSiteMap=new TStringList();                                                 //Steven 20140510 : Secs Gem
    for(int i=0; i<MAX_SOCKET_ROW; i++)
    {
        for(int j=0; j<MAX_SOCKET_COL; j++)
        {
            tSiteMap->Add("0");
        }
    }

    // AI(W906-FW-SETUP-D) 20260824: DEVIATION D-1 -- real VCL TRadioGroup
    // allocates Items in its ctor; the shared vclcompat base deliberately left
    // it NULL, so golden's Clear()/Add() below would deref NULL without an
    // explicit `rgInOutArmYPitch->Items=new TStringList();` here.
    // AI(W906-FW3-Observer-W2) 20260825: DEVIATION D-1 RETIRED -- vclcompat::
    // TRadioGroup now allocates Items in its own constructor and frees it in
    // its destructor (Controls.h:431-437), matching real VCL and the
    // TComboBox/TListBox already there. Keeping this line would LEAK the
    // constructor's list, so it is removed rather than left as a no-op.
    if(USE_IN_OUT_ARM_Y_PITCH==iXPitchManual360)                                //Steven 20140819 : Y-Pitch 36mm
    {
        rgInOutArmYPitch->Items->Clear();
        rgInOutArmYPitch->Items->Add("60.0 mm");
        rgInOutArmYPitch->Items->Add("36.0 mm");
    }

    for(int i=0; i<iSnSocketCnt; i++)                                           //Steven 20200610 : Socket sensor 改成16顆
    {
        MyTempRGBox[i]=new TfSetupSensorRadioGroup();                           //AI(W906-FW-SETUP-D) 20260824: DEVIATION D-2 -- golden `new TRadioGroup(this)`; Owner dropped (VCL glue), facade type substituted (Items self-allocated in its ctor, D-1)
        MyTempRGBox[i]->Parent=scrlbxSocketSensor;
        MyTempRGBox[i]->Name=AnsiString("rgSensor")+AnsiString(i+1);
        MyTempRGBox[i]->Height=36;
        Str.sprintf("Sensor %d usage", i+1);
        MyTempRGBox[i]->Caption=Str;
        MyTempRGBox[i]->Columns=3;
        MyTempRGBox[i]->Items->Add("No use");
        MyTempRGBox[i]->Items->Add("Has IC");
        MyTempRGBox[i]->Items->Add("Floating");
        // AI(W906-FW-SETUP-D) 20260824: GATE(G-Align) -- golden :213
        // `MyTempRGBox[i]->Align=alTop;` -- no Align port on any vclcompat
        // control (rendering = web; forms/fTemp_Set.h's established gate).
#if 0 // GATE(G-Align)
        MyTempRGBox[i]->Align=alTop;
#endif // GATE(G-Align)
        // AI(W906-FW-SETUP-D) 20260824: GATE(G-Delegate) -- golden :214
        // `MyTempRGBox[i]->OnClick=rgSensor1Click;` -- stock vclcompat
        // widgets carry no OnClick delegate slot (forms/fTemp_Set.h's
        // established gate; the OnChange/OnScroll slots on the new
        // vclcompat::TScrollBar are the user-ruled exception, on that type
        // only). The future web write-path wiring calls
        // fSetup->rgSensor1Click(box) directly.
#if 0 // GATE(G-Delegate)
        MyTempRGBox[i]->OnClick=rgSensor1Click;
#endif // GATE(G-Delegate)
    }
}

// ---------------------------------------------------------------------------
// TfSetup::FormDestroy
//   BCB6 source: cSetUp.cpp:218-230 (13 lines)
//   golden `TObject *Sender` dropped, never read. MyDBIProcess: real decl
//   aHotPlateSubstrate.h:933 (already included); LogSoftwareOffTime: decl
//   cmydef.h:5032, offline no-op body acarry_shims.cpp:255.
//   NOTE: dereferences tSiteMap, which only Init() allocates -- golden could
//   never run FormDestroy before its ctor; the wiring wave owns that order.
// ---------------------------------------------------------------------------
void TfSetup::FormDestroy()
{
    try
    {
        tSiteMap->Clear();                                                      //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete tSiteMap;                                                        //Steven 20140510 : Secs Gem
    }
    catch(...)
    {
        MyDBIProcess("Exception", "TfSetup::FormDestroy");
    }
    LogSoftwareOffTime("TfSetup, FormDestroy");                                 //Steven 20210526 : 紀錄軟體執行時間
}

// ---------------------------------------------------------------------------
// TfSetup::CompChange
//   BCB6 source: cSetUp.cpp:1195-1262 (68 lines)
// ---------------------------------------------------------------------------
void TfSetup::CompChange(int iMode)
{
    int iTestCHCT=SiteData[iMode].Cnt;

    if(IniConfig.bDualSiteSupply4CH==true)                                      //jou 2012-11-20 Dual Site supply 4's Channel
    {
        if(iMode==DualSite)
        {
            iTestCHCT+=2;
        }
    }

    if(IniConfig.bSPILFunction==true ||                                         //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
       CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        if(iMode==_8Site2X4 &&
           CosFunction.bEnableOctal_12Kit==true &&
           cbOctal12Site->Checked==true)
        {
            iTestCHCT+=4;
        }
        else if(iMode==_8Site2X4 &&
                CosFunction.bEnableOctal_12Kit==true &&
                cbOctal12Site->Checked==false)
        {
            iTestCHCT=SiteData[iMode].Cnt;
        }
    }

    if(CosFunction.bUse32ChanelSiteMap)                                         //Steven 20170530 (wei) : Use 32CH site map
    {
        iTestCHCT=32;

        if(IniConfig.bVTESTFunction)                                            //RogerYang 20260529 : 偉測趙坤鵬說開放到最多16site
        {
            iTestCHCT=16;
        }
    }

    for(int i=0; i<MAX_SOCKET_ROW; i++)
    {
        TestLabRow[i]->Visible=false;
        for(int j=0; j<MAX_SOCKET_COL; j++)
        {
            TestSiteCH[i][j]->Clear();
            TestSiteCH[i][j]->Visible=false;
            TestLabCol[j]->Visible=false;
        }
    }

    for(int i=0; i<SiteData[iMode].XItem; i++)
    {
        TestLabCol[i]->Visible=true;
        for(int j=0; j<SiteData[iMode].YItem; j++)
        {
            TestSiteCH[j][i]->Visible=true;
            TestSiteCH[j][i]->Enabled=true;
            for(int k=0; k<=iTestCHCT; k++)
            {
                if(k==0)
                    TestSiteCH[j][i]->Items->Add("- - -");
                else
                    TestSiteCH[j][i]->Items->Add("CH "+AnsiString (k));
            }
            TestLabRow[j]->Visible=true;
        }
    }
}

// ---------------------------------------------------------------------------
// TfSetup::CHSetError
//   BCB6 source: cSetUp.cpp:1284-1605 (322 lines)
//   GOLDEN QUIRK (:1559/:1590): `if(iYpitch>63.5)` -- int compared against
//   63.5, behaviourally identical to >=64. Verbatim.
//   GOLDEN ODDITY: the switch has no case for QualSite2X2N/_6Site2X3N/
//   _8Site2X4N/_32Site4X8N and no default -- those modes' pitches leave this
//   validator entirely unchecked. Verbatim.
// ---------------------------------------------------------------------------
bool TfSetup::CHSetError()
{
    bool bSelOne=false;
    int iXpitch, iYpitch;
    int iRow1, iCol1, iRow2, iCol2;

    for(int i=0; i<MAX_SOCKET_TOTAL; i++)
    {
        for(int j=i+1; j<MAX_SOCKET_TOTAL; j++)
        {
            iRow1=i/MAX_SOCKET_COL;
            iCol1=i%MAX_SOCKET_COL;
            iRow2=j/MAX_SOCKET_COL;
            iCol2=j%MAX_SOCKET_COL;
            if(TestSiteCH[iRow1][iCol1]->ItemIndex==-1 &&
               TestSiteCH[iRow1][iCol1]->Visible)
            {
                ShowMyMessage("Test Site Not Select");
                return true;
            }

            if(TestSiteCH[iRow1][iCol1]->Visible &&
               TestSiteCH[iRow2][iCol2]->Visible)
            {
                if(TestSiteCH[iRow1][iCol1]->ItemIndex==TestSiteCH[iRow2][iCol2]->ItemIndex &&
                   TestSiteCH[iRow1][iCol1]->ItemIndex!=0)
                {
                    ShowMyMessage("Test Site Set Same CH");
                    return true;
                }

                if(TestSiteCH[iRow1][iCol1]->ItemIndex>=1 ||
                   TestSiteCH[iRow2][iCol2]->ItemIndex>=1)
                {
                    bSelOne=true;
                }
            }
        }
    }

    if(ScrollBar1->Position==SingleSite)
    {
        TestSiteCH[0][0]->ItemIndex=1;
        bSelOne=true;
    }

    if(bSelOne==false)
    {
        ShowMyMessage("Test Site Not Select");
        return true;
    }

    iXpitch=atoi(XPitch->Text.c_str());                                         //jou 980725 start : set over limit
    iYpitch=atoi(YPitch->Text.c_str());

    if(ScrollBar1->Position!=SingleSite &&
       ScrollBar1->Position!=DualSite2x1)
    {
        if(iXpitch<20)
        {
            ShowMyMessage("X Pitch can not < 20mm!!");
            return true;
        }
    }

    switch(ScrollBar1->Position)
    {
        case SingleSite:
            break;
        case DualSite2x1:
            if(iYpitch>80)
            {
                ShowMyMessage("Y Pitch can not over 80mm");
                return true;
            }

            if(iYpitch<20)
            {
                ShowMyMessage("Y Pitch can not < 20mm");
                return true;
            }
            break;
        case DualSite:                                                          //1x2
            if(MachineTypeChoice==Type_HT9046_LS)                               //Steven 20200114 : SCK新的SLK
            {
                if(iXpitch<40 || iXpitch>240)
                {
                    ShowMyMessage("X Pitch Must Between 40~240mm");
                    return true;
                }
            }
            else if(USE_IN_OUT_ARM_X_PITCH==iXPitch40mm)                        //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
            {
                if(iXpitch<40 || iXpitch>120)                                   //Steven 20150505 : 1x2加大支援X-Pitch 120mm
                {
                    ShowMyMessage("X Pitch Must Between 40~120mm");
                    return true;
                }
            }
            else
            {
                if(iXpitch<40 || iXpitch>150)
                {
                    ShowMyMessage("X Pitch Must Between 40~150mm");
                    return true;
                }
            }
            break;
        case QualSite1X4:                                                       //1x4
        case _8Site1X4:                                                         //ChungHung 20150528 add for 海思 _8Site1x4
            if(USE_IN_OUT_ARM_X_PITCH==iXPitch40mm)                             //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
            {
                if(iXpitch>80)                                                  //Steven 20100617 : 60 -> 80
                {
                    ShowMyMessage("X Pitch can not over 80mm");
                    return true;
                }
            }
            else
            {
                if(iXpitch>100)
                {
                    ShowMyMessage("X Pitch can not over 100mm");
                    return true;
                }
            }
            break;
        case QualSite2X2:                                                       //2x2
            if(iXpitch>200)                                                     //Steven 20150224 : 韓國Danny說要180的Pitch
            {
                ShowMyMessage("X Pitch can not over 200mm");
                return true;
            }

            if(iYpitch>80)
            {
                ShowMyMessage("Y Pitch can not over 80mm");
                return true;
            }

            if(iYpitch<20)
            {
                ShowMyMessage("Y Pitch can not < 20mm");
                return true;
            }
            break;
        case _6Site2X3:                                                         //ChungHung 20140115 add for 2x3_6
            if(USE_IN_OUT_ARM_X_PITCH==iXPitch40mm)                             //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
            {
                if(iXpitch>120)                                                 //Steven 20200211 : 80 --> 120 for SCK
                {
                    ShowMyMessage("X Pitch can not over 120mm");
                    return true;
                }
            }
            else
            {
                if(iXpitch>100)
                {
                    ShowMyMessage("X Pitch can not over 100mm");
                    return true;
                }
            }

            if(iYpitch>80)
            {
                ShowMyMessage("Y Pitch can not over 80mm");
                return true;
            }

            if(iYpitch<20)
            {
                ShowMyMessage("Y Pitch can not < 20mm");
                return true;
            }
            break;
        case _8Site2X4:                                                         //2x4
        case _16Site4X4:                                                        //Sam 20190226 : 16Site4X4
            if(USE_IN_OUT_ARM_X_PITCH==iXPitch40mm)                             //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
            {
                if(iXpitch>80)                                                  //Steven 20100825 : 60 -> 80
                {
                    ShowMyMessage("X Pitch can not over 80mm");
                    return true;
                }
            }
            else
            {
                if(iXpitch>100)
                {
                    ShowMyMessage("X Pitch can not over 100mm");
                    return true;
                }
            }

            if(iYpitch>80)                                                      //Steven 20100617 : 60 -> 80
            {
                ShowMyMessage("Y Pitch can not over 80mm");
                return true;
            }

            if(iYpitch<20)
            {
                ShowMyMessage("Y Pitch can not < 20mm");
                return true;
            }
            break;
        case _10Site2X5:                                                        //wei 20190614 10 site
        case _12Site2X6:
            if(USE_IN_OUT_ARM_X_PITCH==iXPitch40mm)                             //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
            {
                if(iXpitch>80)
                {
                    ShowMyMessage("X Pitch can not over 80mm");
                    return true;
                }
            }
            else
            {
                if(iXpitch>100)
                {
                    ShowMyMessage("X Pitch can not over 100mm");
                    return true;
                }
            }

            if(iYpitch>80)
            {
                ShowMyMessage("Y Pitch can not over 80mm");
                return true;
            }

            if(iYpitch<30)                                                      //Steven 20170530 (wei) : 35 --> 30
            {
                ShowMyMessage("Y Pitch can not < 30mm");
                return true;
            }
            break;
        case _16Site2X8:                                                        //2x8
            if(USE_IN_OUT_ARM_X_PITCH==iXPitch50mm)                             //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
            {
                if(iXpitch>50)
                {
                    ShowMyMessage("X Pitch can not over 50mm");
                    return true;
                }
            }
            else
            {
                if(MachineTypeChoice==Type_HT9046_LS ||
                   SubMachineType==Type_HT9046LA)                               //Steven 20190307 : Add for HT-9046LA
                {
                    if(iXpitch>45)                                              //JerryYang 20171101 (wei) 16 site X-pitch最大值40->45
                    {
                        ShowMyMessage("X Pitch can not over 45mm");
                        return true;
                    }
                }
                else if(MachineTypeChoice==Type_HT1032)
                {
                    if(iXpitch>40)
                    {
                        ShowMyMessage("X Pitch can not over 40mm");
                        return true;
                    }
                }
                else
                {
                    if(iXpitch>30)
                    {
                        ShowMyMessage("X Pitch can not over 30mm");
                        return true;
                    }
                }
            }

            if(iYpitch>63.5)
            {
                ShowMyMessage("Y Pitch can not over 63.5mm");
                return true;
            }

            if(iYpitch<20)
            {
                ShowMyMessage("Y Pitch can not < 20mm");
                return true;
            }
            break;
        case _32Site4X8M:
            if(USE_IN_OUT_ARM_X_PITCH==iXPitch40mm ||                           //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
               MachineTypeChoice==Type_HT1032)
            {
                if(iXpitch>40)
                {
                    ShowMyMessage("X Pitch can not over 40mm");
                    return true;
                }
            }
            else
            {
                if(iXpitch>50)
                {
                    ShowMyMessage("X Pitch can not over 50mm");
                    return true;
                }
            }

            if(iYpitch>63.5)
            {
                ShowMyMessage("Y Pitch can not over 63.5mm");
                return true;
            }

            if(iYpitch<35)
            {
                ShowMyMessage("Y Pitch can not < 35mm");
                return true;
            }
            break;
    }
    return false;
}

// ---------------------------------------------------------------------------
// TfSetup::rgUseSuckModeClick
//   BCB6 source: cSetUp.cpp:3423-3450 (28 lines)
//   golden `TObject *Sender` dropped, never read.
// ---------------------------------------------------------------------------
void TfSetup::rgUseSuckModeClick()
{
    int iPos=ScrollBar1->Position;

    switch(iPos)
    {
        case DualSite:
        case QualSite1X4:
        case _8Site1X4:                                                         //ChungHung 20150528 add for 海思 _8Site1x4
            if(rgUseSuckMode->ItemIndex==1)
            {
                if(IniConfig.bDisableSelectSearchLast==true)                    //jou 2012-01-10 取消Setup，Search Last Mode功能。
                {
                    rgSelectSearchLast->Visible=false;
                    rgSelectSearchLast->ItemIndex=0;
                }
                else
                {
                    rgSelectSearchLast->Visible=true;
                }
            }
            else
            {
                rgSelectSearchLast->Visible=false;
            }
            break;
    }
}

// ---------------------------------------------------------------------------
// TfSetup::CheckShuttlePitch
//   BCB6 source: cSetUp.cpp:4645-4706 (62 lines)                               //wei 20160914 Auto Shuttle Sensor
// ---------------------------------------------------------------------------
bool TfSetup::CheckShuttlePitch()                                               //wei 20160914 Auto Shuttle Sensor
{
    bool bShuttleSensorCanMove=false;
    double iShuttlePitch=0.0;
    double iXpitch=atof(XPitch->Text.c_str());

    if(AUTO_SENSOR_INSTALL)
    {
        if(ScrollBar1->Position==DualSite     ||                                //1x2 //sensor 2 4
           ScrollBar1->Position==QualSite2X2  ||
           ScrollBar1->Position==QualSite2X2N)                                  //2x2
        {
            iShuttlePitch=iXpitch*100;
            if(iShuttlePitch*3>=12000 && iShuttlePitch*3<=24000)
            {
                bShuttleSensorCanMove=true;
            }
            else if(iShuttlePitch>=9000 && iShuttlePitch<=18000)
            {
                bShuttleSensorCanMove=true;
            }
        }
        else if(ScrollBar1->Position==TriSite1X3 ||                             //1x3  //Frank 20160329 add for 1x3_4    //sensor 1 3 5
                ScrollBar1->Position==_6Site2X3  ||                             //ChungHung 20140115 add for 2x3_6
                ScrollBar1->Position==_6Site2X3N )                              //Steven 20220425 : 2x3 NN mode
        {
            iShuttlePitch=iXpitch*2*100;
            if(iShuttlePitch*3>=12000 && iShuttlePitch*3<=24000)
            {
                bShuttleSensorCanMove=true;
            }
            else if(iShuttlePitch>=8759 && iShuttlePitch<=24000)                //20170213
            {
                bShuttleSensorCanMove=true;
            }
        }
        else if(ScrollBar1->Position==QualSite1X4 ||                            //1x4                       //sensor 1 2 4 5
                ScrollBar1->Position==_8Site1X4   ||                            //ChungHung 20150528 add for 海思 _8Site1x4
                ScrollBar1->Position==_8Site2X4   ||
                ScrollBar1->Position==_16Site4X4  ||                            //Sam 20190226 : 16Site4X4 //2x4
                ScrollBar1->Position==_8Site2X4N  )                             //Wei 20231211 : 2X4NN Mode
        {
            iShuttlePitch=iXpitch*3*100;
            if(iShuttlePitch>=12000 && iShuttlePitch<=24000)
            {
                bShuttleSensorCanMove=true;
            }
        }

        if(bShuttleSensorCanMove==false)
        {
            ShowMyMessage("Auto Shuttle Sensor 不支援此Pitch!!");
            return false;
        }
    }
    else
    {
        return true;
    }

    return bShuttleSensorCanMove;
}

// ---------------------------------------------------------------------------
// TfSetup::CoSocketComboChange
//   BCB6 source: cSetUp.cpp:4752-4770 (19 lines)
//   golden `TObject *Sender` dropped -- GOLDEN ODDITY: golden itself ignores
//   Sender and reads CoSocketCombo directly (see this file's Wave D banner).
// ---------------------------------------------------------------------------
void TfSetup::CoSocketComboChange()
{
    for(int i=0; i<iSnSocketCnt; i++)                                           //Steven 20200420 : Socket Sensor功能可以選
    {
        if(i<atoi(CoSocketCombo->Text.c_str()))
        {
            MyTempRGBox[i]->Visible=true;
            if(TestIF_File.bSocketSensorCheckFloating)
                MyTempRGBox[i]->ItemIndex=2;
            else
                MyTempRGBox[i]->ItemIndex=1;
        }
        else
        {
            MyTempRGBox[i]->Visible=false;
            MyTempRGBox[i]->ItemIndex=0;
        }
    }
}

// ============================================================================
//  FW-SETUP-E (20260824) -- the fQwertyKey/fPassword unlock batch
//  --------------------------------------------------------------------------
//  AI(W906-FW-SETUP-E) 20260824: three queued methods whose blocker
//  ("TfQwertyKey/TfPassword have no compiled entity") died with FW-QWKEY1
//  (fc08e09, forms/fQwertyKey.* + forms/fPassword.* wired into the build).
//  Golden file-scope globals :55-56 land here too, spelled explicit `static`
//  per the extern-flip policy (golden writes them without static; golden has
//  no extern for either -- single-file, verified 20260824). The remaining
//  golden file-scope globals (:53-54 iASMTestMode/iASMSiteMap, :121
//  OrgTestMode) stay undefined with their still-queued consumers.
//
//  GATE (E-B1)/(E-B2): ⚠ AI(W906-FW-SETUP-W19) 20260826 -- **整段已過期**。
//  兩個 gate 都在 20260825（FW-BARCODE2）開掉了，見本檔 XPitchMouseDown /
//  XShiftPitchMouseDown 各自的 OPENED 註記。Barcode_Reader 的真本體在
//  BarcodeReader.cpp:445（golden :415-444），下面那句「NO port tree-wide」
//  是 20260824 的量測，當時為真、現在為假。原文保留為沿革：
//  the Barcode_Reader(bcSetup) early-return guards in
//  XPitchMouseDown/XShiftPitchMouseDown -- Barcode_Reader has NO port
//  tree-wide (re-verified 20260824: Grep for an int Barcode_Reader definition
//  over *.h/*.cpp = 0 hits; the only other caller, MyTempPanel.cpp:840, sits
//  inside its own #if 0). Gating the guard drops its early-return: on a KYEC
//  barcode-reader machine golden can REFUSE the keyboard popup; this port
//  never refuses. Latent today -- the handlers are not wired to any event
//  source.
// ============================================================================
static bool bNeedEnterPassword=false;                                           //JerryYang 20160425 修改Site map需要密碼
static AnsiString sSigPassword;                                                 //Alick 20160602 矽格Password用
//------------------------------------------------------------------------------
//AI(W906-FW-SETUP-E) 20260824: golden :3409-3421. D-3 glue params dropped
// (Button/Shift/X/Y unread); Sender typed TEdit* (golden casts (TEdit*)Sender,
// forms/fQwertyKey.cpp spbKeyClick precedent). fQwertyKey deref is faithful
// golden shape (NULL until a wiring wave constructs it -- G-d exposure).
void TfSetup::XPitchMouseDown(TEdit *Sender)
{
    // GATE (E-B1) OPENED 20260825 (FW-BARCODE2): Barcode_Reader real since
    // FW-BARCODE1 (e7b4bf8); non-KYEC config returns 2, guard never fires.
    if(Barcode_Reader(bcSetup)==0)                                              // 20140103 wei KYEC Barcode Reader
    {
        return;
    }

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
        fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, 0.0, 1000.0);
    else
        fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 3, true, 0.0, 1000.0);
}
//------------------------------------------------------------------------------
//AI(W906-FW-SETUP-E) 20260824: golden :4738-4750, byte-identical twin of
// XPitchMouseDown above except the CheckRange bounds (-4000..4000).
void TfSetup::XShiftPitchMouseDown(TEdit *Sender)
{
    // GATE (E-B2) OPENED 20260825 (FW-BARCODE2) -- see E-B1 above.
    if(Barcode_Reader(bcSetup)==0)
    {
        return;
    }

    if(IniConfig.bSPILFunction==true)
        fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, -4000.0, 4000.0);
    else
        fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 3, true, -4000.0, 4000.0);
}
//------------------------------------------------------------------------------
//AI(W906-FW-SETUP-E) 20260824: golden :4408-4441 verbatim. Sender typed
// TComboBox* (golden casts (TComboBox*)Sender); the local Ptr kept so the
// body reads like golden. SetFocus is the new TControl offline no-op
// (vclcompat/Controls.h, this wave).
void TfSetup::cbAaDropDown(TComboBox *Sender)
{
    if(IniConfig.bG09NeedPasswordWhenEditSiteMap==true)                         //Alick 20160603 Site Map 需要密碼
    {
        if(bNeedEnterPassword==true)
        {
            fPassword->edPassword->Text="";
            fQwertyKey->ShowQwertyKey(fPassword->edPassword, N_NO_SYMBOL|N_NO_SPACE|N_PASSWORD);
            if(sSigPassword==fPassword->edPassword->Text)
            {
                bNeedEnterPassword=false;
            }
            else
            {
                if(TestIF_File.iTestMode==SingleSite)                           //Alick 20161216 add modify for PeiXing 用鍵盤選擇SITE別沒輸入密碼卻可以變更的Bug
                    ScrollBar1->SetFocus();
                else
                    XPitch->SetFocus();
                return;
            }
        }
    }
    else
    {
        bNeedEnterPassword=false;
    }

    if(bNeedEnterPassword==false)
    {
        TComboBox *Ptr;
        Ptr=Sender;
        SitCH=Ptr->ItemIndex;
    }
}

//------------------------------------------------------------------------------
//AI(W906-FW-SETUP-W19) 20260826: golden cSetUp.cpp:4451-4462，逐字翻譯。
// 本方法先前被本檔 :236 的表列為 deferred，理由只有一個：「Barcode_Reader」。
// **那個理由已經不成立**——Barcode_Reader 的真本體在 BarcodeReader.cpp:445
// （golden :415-444），FW-BARCODE1 就翻好了，同檔的 E-B1/E-B2 兩個 gate 也
// 早在 20260825 因此開掉。所以本波把它翻進來，不加任何 gate。
//
// 簽章保留 golden 原文（TMouseButton/TShiftState 自 vclcompat/ShiftState.h
// commit f184093 起可拼寫）。
void TfSetup::rgShtModeNormalMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Sender; (void)Button; (void)Shift; (void)X; (void)Y;   //AI(W906-FW-SETUP-W19): golden 也沒讀
    if(Barcode_Reader(bcSetup)==0)                                              // 20140103 wei KYEC Barcode Reader
    {
        if(TestIF_File.iShuttleMode==0)
            rgShtModeNormal->Checked=true;
        else
            rgShtModeOneSide->Checked=true;
        return;
    }
}
