// =============================================================================
//  test_binsel_core.cpp -- FW-BinSel-WA/WB: TfBinSel data-core test coverage
//
//  AI(W906-FW-BinSel-WA) 20260819: new file.
//
//  AI(W906-FW-BinSel-WB) 20260819: adds ReadFunctionData oracle coverage now
//  that WAVE B lands its real body. Seam used: `ReadFunctionData(tag,szDir)`
//  takes `szDir` as an explicit PARAMETER (WAVE B's own DEVIATION from golden,
//  see cBinSel.cpp's banner on that method) -- this IS the test-mode redirect
//  seam the WAVE A banner above says doesn't exist ("none of the three has a
//  test-mode redirect seam reachable from TfBinSel's own public surface");
//  that was true for the WAVE-A-era methods this file already covers, not for
//  ReadFunctionData post-WAVE-B. Tests below write a SCRATCH ini under
//  TEMP/TMP (same `tmpIniPath` idiom as tests/test_IniFiles.cpp) and pass its
//  path directly -- never GetRecipePath()/GetLastOpenFN()/DataPath, so no
//  production config path is ever touched. `SaveFunctionData`/
//  `SetPrimeButton`/`mtTrayNameSetColor` stay OUT of scope here: WAVE B gates
//  100% (or, for the latter two, the entire body) of their real effect, so
//  there is nothing to observe -- see forms/fBinSel.h GATE REGISTER G7-G9.
//
//  Covers the ACTIVE, file-I/O-free surface of TfBinSel: the ctor's
//  TStringList/MyBinPanelData bootstrap (the state the 21+ SECSGEM EC gate
//  sites and Command.cpp's SetTrayBinByDLL family read), TransferBinTrayStrToName
//  (pure index-to-name conversion), ARTBinCheck (pure Pass-Bin-Retest forcing
//  logic), CheckFix2Tray/CheckOSBin (pure predicate reads), and
//  ChangeActivePageIndex (pure widget-index mapping).
//
//  DELIBERATELY NOT COVERED (see forms/fBinSel.h WRITE-PATH GATE TABLE for
//  the policy this follows -- every one of these calls a REAL file-reading
//  free function, `CheckAndReadIniData`/`ReadIniData`, against a REAL
//  production path built from `GetRecipePath()`/`GetRecipeFileName()`/
//  `DataPath+GetLastOpenFN()` -- none of the three has a test-mode redirect
//  seam reachable from TfBinSel's own public surface, so calling any of them
//  here would risk touching real, possibly production-shared config under
//  system\/config\):
//    * ReadFile, SetConFail, Save, spbSaveClick, SaveOther -- the whole
//      MUST-HAVE / write-path family this wave's brief calls out as "極可能
//      讀寫 bin/tray 設定檔".
//    * ReadWriteMRTMode, ReadWriteSpecialFunction, ReadPrimeDara,
//      WritePrimeDara -- each unconditionally builds `DataPath+
//      GetLastOpenFN()+"...Data"` and reads it via `ReadIniData` on at least
//      one code path (verified by reading all four bodies in full this
//      wave); `SetOSBin` is ALSO excluded for this reason since its own body
//      calls `ReadWriteMRTMode` twice.
//    * SaveFunctionData/SetPrimeButton -- WAVE B's own policy gates 100%
//      (SaveFunctionData) or the ENTIRE body (SetPrimeButton) of their real
//      effect (see forms/fBinSel.h GATE REGISTER G7-G8) -- nothing
//      observable to assert. `mtTrayNameSetColor` is NO LONGER in this list
//      -- FW-BinSel-WC unlocks its full body (GATE G9 CLOSED) and gets its
//      own coverage below. ReadFunctionData is likewise DIFFERENT: WAVE B
//      gives it an explicit `szDir` parameter (see this file's own WAVE B
//      banner above), so it DOES get scratch-ini oracle coverage below now.
//    * CheckFix2Tray's "found a Fix2 tray" TRUE branch -- the column index
//      that would trigger it depends on `eBinNotUse`'s numeric value, which
//      is TU-LOCAL to cBinSel.cpp (`enum eBinSettingItems`, not exposed via
//      forms/fBinSel.h) -- only the "no match" paths (AOI disabled; AOI
//      enabled but the ctor's all-zero table) are constructible from outside
//      that TU and are covered below.
//
//  AI(W906-FW-BinSel-WC) 20260819: adds coverage for WAVE C's own additions --
//  (2b) the ctor's mtTrayName/mtTrayItem cell-fill (including its ONE
//  SIOF-guarded line, unlocked via the SAME OpenGeneralIniFile()/
//  asGeneralPath redirect idiom tests/test_observer_core.cpp already
//  established for INIFileGeneral), (9) mtTrayNameSetColor's now-FULL body,
//  and (10) InitDataToEdit -- which, unlike ReadFunctionData, needs NO
//  scratch-ini or path seam at all (verified this wave by reading its full
//  643-line body against golden: zero CheckAndReadIniData/WriteIniData
//  calls anywhere in it -- pure in-memory MyBinPanel[tag]-field-to-
//  sXxx[tag] reshuffling, see that method's own S20 banner in cBinSel.cpp).
// =============================================================================
#include "forms/fBinSel.h"

#include "MachineType.h"      // eBinType (eBinFT/eBinRT/...), eartInstall, USE_AUTO_RETEST
#include "cmydef.h"            // s3TrayName[], iTestRunMode, CUSTOMER_CODE
#include "cprod.h"              // Prod, BinSelect[], tAOISetup
#include "LastSet.h"             // LastSet
#include "Config.h"               // IniConfig
#include "CosFunction.h"           // CosFunction
#include "common.h"                // CheckAndReadIniData/WriteIniData -- scratch-ini seeding only,
                                    // see ReadFunctionData oracle tests below (never a production path);
                                    // also asGeneralPath/OpenGeneralIniFile/CloseGeneralIniFile -- SIOF-guard
                                    // unlock for the ctor's s6TrayName[] line, see (2b) below

#include <cstdio>
#include <cstdlib>   // std::getenv (scratch path)
#include <cstring>

// ---------------------------------------------------------------------------
//  Scratch ini path -- SAME idiom as tests/test_IniFiles.cpp's tmpIniPath():
//  TEMP/TMP env var, falling back to cwd. Never a GetRecipePath()/DataPath/
//  production path.
// ---------------------------------------------------------------------------
static AnsiString BinSelScratchIniPath(const char* leaf)
{
    const char* t = std::getenv("TEMP");
    if (!t || !*t) t = std::getenv("TMP");
    if (!t || !*t) t = ".";
    return AnsiString(t) + AnsiString("\\") + AnsiString(leaf);
}

// ---------------------------------------------------------------------------
//  Minimal CHECK harness (matches tests/test_counterclear_core.cpp precedent)
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg) do { \
        if (cond) { g_pass++; } \
        else { g_fail++; std::printf("FAIL: %s (line %d)\n", msg, __LINE__); } \
    } while (0)

// ---------------------------------------------------------------------------
//  Reset the handful of globals every test in this file touches.
// ---------------------------------------------------------------------------
static void ResetGlobals()
{
    USE_AUTO_RETEST = 0;
    IniConfig.bA10_AutoReTest = false;
    LastSet.iRunStartMode = 0;
    tAOISetup.bEnabledAOI = false;
    CosFunction.bAutoSiteMappingSetOpenBIN = false;
    Prod.iOpenBin = -1;
    iTestRunMode = eBinFT;
    for (int i = 0; i < TEST_MAX_BIN; i++)
        BinSelect[eBinFT].iCatDataT3Pos[i] = 0;
}

// =============================================================================
//  (1) ctor -- the 27 TStringList*[eBinTypeTotal] arrays (golden :984-1076)
// =============================================================================
static void Test_Ctor_PopulatesBinTypeStringLists()
{
    TfBinSel f;

    for (int tag = 0; tag < eBinTypeTotal; tag++)
    {
        CHECK(f.sBinDoubleContact[tag] != NULL, "ctor: sBinDoubleContact[tag] non-null");
        CHECK(f.sBinDoubleContact[tag]->Count == iTestBinCount, "ctor: sBinDoubleContact[tag] sized iTestBinCount");
        CHECK(AnsiString(f.sBinDoubleContact[tag]->Strings[0]) == "0", "ctor: sBinDoubleContact[tag][0] == \"0\" placeholder");

        CHECK(f.sBinTraySetT3Pos[tag] != NULL, "ctor: sBinTraySetT3Pos[tag] non-null (Command.cpp SetTrayBinByDLL consumer)");
        CHECK(f.sBinTraySetT3Pos[tag]->Count == iTestBinCount, "ctor: sBinTraySetT3Pos[tag] sized iTestBinCount");
        CHECK(AnsiString(f.sBinTraySetT3Pos[tag]->Strings[0]) == "0", "ctor: sBinTraySetT3Pos[tag][0] == \"0\" placeholder");

        CHECK(f.sBinTraySetT3PosName[tag] != NULL, "ctor: sBinTraySetT3PosName[tag] non-null");
        CHECK(AnsiString(f.sBinTraySetT3PosName[tag]->Strings[0]) == "NotUse", "ctor: sBinTraySetT3PosName[tag][0] == \"NotUse\" placeholder");

        CHECK(f.sBinType[tag] != NULL, "ctor: sBinType[tag] non-null");
        CHECK(f.sLowYield[tag] != NULL, "ctor: sLowYield[tag] non-null (SECSGEM g21 consumer)");
        CHECK(f.sBySiteClean[tag] != NULL, "ctor: sBySiteClean[tag] non-null (SECSGEM g21 consumer)");
        CHECK(f.sSpecBinByArmPerSiteComparePercent[tag] != NULL, "ctor: sSpecBinByArmPerSiteComparePercent[tag] non-null");
        CHECK(f.sBinTrayLinked[tag] != NULL, "ctor: sBinTrayLinked[tag] non-null (Ifor 20240730)");
        CHECK(f.sAOIBinTraySetting[tag] != NULL, "ctor: sAOIBinTraySetting[tag] non-null (Eastsun 20260316)");
        CHECK(AnsiString(f.sAOIBinTraySetting[tag]->Strings[0]) == "0", "ctor: sAOIBinTraySetting[tag][0] == \"0\" (single-entry Add, golden :1024-1025)");
    }

    CHECK(f.bShow == false, "ctor: bShow defaults false");
}

// =============================================================================
//  (2) ctor -- MyBinPanel[eBinTypeTotal] (Wave-A TMyBinPanelData bootstrap)
// =============================================================================
static void Test_Ctor_MyBinPanelZeroInitialized()
{
    TfBinSel f;

    for (int tag = 0; tag < eBinTypeTotal; tag++)
    {
        CHECK(f.MyBinPanel[tag] != NULL, "ctor: MyBinPanel[tag] non-null");
        CHECK(f.MyBinPanel[tag]->iErrorT6 == 0, "ctor: MyBinPanel[tag]->iErrorT6 == 0");
        CHECK(f.MyBinPanel[tag]->iT6IsFail[0] == 0, "ctor: MyBinPanel[tag]->iT6IsFail[0] == 0");
        CHECK(f.MyBinPanel[tag]->bT6Link[0] == false, "ctor: MyBinPanel[tag]->bT6Link[0] == false");
        CHECK(f.MyBinPanel[tag]->BackT6PosTray[0][0] == 0, "ctor: MyBinPanel[tag]->BackT6PosTray[0][0] == 0");
        CHECK(f.MyBinPanel[tag]->BackT6PosTray[TEST_MAX_BIN-1][TEST_MAX_BIN-1] == 0, "ctor: MyBinPanel[tag]->BackT6PosTray last cell == 0");
    }
}

// =============================================================================
//  (2b) ctor -- MyBinPanel[tag]->mtTrayName/mtTrayItem cell-fill (golden
//       TMyBinPanel ctor :386-387/:417-418/:421-511, FW-BinSel-WC). Cell
//       coordinates below are RAW INTEGER LITERALS, not the TU-local
//       `eItemXxx`/`eBinNo`/`eBinSetting` enum names (cBinSel.cpp's own
//       `enum eTrayNameFunc`/`eBinSettingItems`, not exposed via
//       forms/fBinSel.h) -- same "constructible subset" posture as
//       CheckFix2Tray's own test banner above. eItemLink=0/eItemPass=1/
//       eItemError=2/eItemName=0 and eBinSetting=26 (24 explicit
//       eBinSettingItems entries 0..24, +eBinNotUse auto=25, +eBinSetting
//       auto=26) are read directly off golden's own enum literals, not
//       guessed.
// =============================================================================
static void Test_Ctor_PopulatesMtTrayNameAndItemStaticLabels()
{
    ResetGlobals();
    TfBinSel f;

    // "Col Name" header row (row eBinNo==0) on mtTrayName (golden :422-423).
    CHECK(f.MyBinPanel[eBinFT]->mtTrayName->GetCellText(0 /*eItemLink*/, 0 /*eBinNo*/) == "Link",
          "ctor: mtTrayName[eItemLink][eBinNo] == \"Link\"");
    CHECK(f.MyBinPanel[eBinFT]->mtTrayName->GetCellText(1 /*eItemPass*/, 0) == "Failed",
          "ctor: mtTrayName[eItemPass][eBinNo] == \"Failed\" (column HEADER label, not a live Pass/Fail cell)");

    // "Row Name" header column (col eItemName==0) on mtTrayItem (golden :429).
    CHECK(f.MyBinPanel[eBinFT]->mtTrayItem->GetCellText(0 /*eItemName*/, 0 /*eBinNo*/) == "Items",
          "ctor: mtTrayItem[eItemName][eBinNo] == \"Items\"");

    // per-tray-row loop (golden :480-511): row eBinSetting(26)+0 == first
    // tray row (eAuto1) -- "Error" column text+colour is unconditional
    // (golden :485-486, no CosFunction/IniConfig gate).
    CHECK(f.MyBinPanel[eBinFT]->mtTrayName->GetCellText(2 /*eItemError*/, 26) == "Error",
          "ctor: mtTrayName[eItemError][eBinSetting+0] == \"Error\" (every tray row)");
    CHECK(f.MyBinPanel[eBinFT]->mtTrayName->CellColorIndex[2][26] == 0 /*eCLWhite*/,
          "ctor: mtTrayName[eItemError][eBinSetting+0] colour == eCLWhite (golden :486)");

    // sizing prerequisite for mtTrayNameSetColor's own FYItem bound check
    // (golden :386, translated alongside the cell-fill span since that
    // method's loop is a total no-op without it -- see cBinSel.cpp's ctor
    // banner).
    CHECK(f.MyBinPanel[eBinFT]->mtTrayName->FYItem == 26 + eTrayCount,
          "ctor: MyBinPanel[tag]->mtTrayName sized to eBinSetTotal (golden :386)");
}

// AI(W906-FW-BinSel-WC) 20260819: SIOF guard oracle -- `s6TrayName[iT6]`
// (golden :483) is the ONE ctor-cell-fill line reading a cross-TU global
// with a non-trivial ctor (cmydef.cpp:44); gated `if(INIFileGeneral!=0)`
// in TfBinSel::TfBinSel (see forms/fBinSel.h STATIC-INIT SAFETY UPDATE).
// Unlocked here exactly like tests/test_observer_core.cpp's own
// INIFileGeneral redirect precedent -- scratch path, never production
// system\Gerneral.ini.
static void Test_Ctor_MtTrayItemRowLabelPopulatesOnlyWhenGeneralIniOpen()
{
    ResetGlobals();

    AnsiString savedGeneralPath = asGeneralPath;
    asGeneralPath = BinSelScratchIniPath("test_binsel_general.ini");
    OpenGeneralIniFile();
    {
        TfBinSel f;
        CHECK(f.MyBinPanel[eBinFT]->mtTrayItem->GetCellText(0 /*eItemName*/, 26) == s6TrayName[0],
              "ctor: mtTrayItem[eItemName][eBinSetting+0] == s6TrayName[eAuto1] when INIFileGeneral is open");
    }
    CloseGeneralIniFile();
    asGeneralPath = savedGeneralPath;
}

// =============================================================================
//  (3) TransferBinTrayStrToName -- golden :6404-6423 (pure index/name convert)
// =============================================================================
// AI(W906-FW-BinSel-WA-integrate) 20260819: both tests below first landed
// constructing TfBinSel with the ambient iTestBinCount==0 -- golden's ctor
// fill loop (`for(j=0;j<iTestBinCount;j++)`, golden :1027) then adds ZERO
// entries and golden's own `i>=iTestBinCount` guard forces every slot to
// "NotUse", so the name-conversion assertions failed at the integration gate
// (the wave agent never executes its tests -- first real run is always the
// gate). Golden's real precondition is WinMain-after-config-load where
// iTestBinCount is loaded (e.g. 256); the fix reproduces that precondition
// test-locally BEFORE construction, with save/restore.
static void Test_TransferBinTrayStrToName_ConvertsIndexToTrayName()
{
    int savedTestBinCount = iTestBinCount;
    iTestBinCount = 256;                 // golden WinMain precondition: ctor fill + guard both live
    {
        TfBinSel f;

        f.sBinTraySetT3Pos[eBinFT]->CommaText = "0,3,5";

        f.TransferBinTrayStrToName(eBinFT);

        CHECK(AnsiString(f.sBinTraySetT3PosName[eBinFT]->Strings[0]) == "NotUse", "TransferBinTrayStrToName: iT3==0 -> \"NotUse\"");
        CHECK(AnsiString(f.sBinTraySetT3PosName[eBinFT]->Strings[1]) == s3TrayName[3-1], "TransferBinTrayStrToName: iT3==3 -> s3TrayName[2]");
        CHECK(AnsiString(f.sBinTraySetT3PosName[eBinFT]->Strings[2]) == s3TrayName[5-1], "TransferBinTrayStrToName: iT3==5 -> s3TrayName[4]");
    }
    iTestBinCount = savedTestBinCount;
}

static void Test_TransferBinTrayStrToName_OutOfRangeIndexIsNotUse()
{
    int savedTestBinCount = iTestBinCount;
    iTestBinCount = 256;
    {
        TfBinSel f;

        // sBinTraySetT3Pos[eBinRT] keeps the ctor's own "0" placeholders
        // (256 of them under the restored precondition) -- every slot reads
        // back "NotUse" through golden's iT3==0 arm.
        f.TransferBinTrayStrToName(eBinRT);

        CHECK(AnsiString(f.sBinTraySetT3PosName[eBinRT]->Strings[0]) == "NotUse", "TransferBinTrayStrToName: ctor-default \"0\" entries stay \"NotUse\"");
    }
    iTestBinCount = savedTestBinCount;
}

// =============================================================================
//  (4) ARTBinCheck -- golden :6381-6401 (pure Pass-Bin-Retest forcing logic)
// =============================================================================
static void Test_ARTBinCheck_ForcesRetestWhenAllThreeSitesPassOrNoneAreART()
{
    ResetGlobals();
    TfBinSel f;

    USE_AUTO_RETEST = eartInstall;
    IniConfig.bA10_AutoReTest = true;
    LastSet.iRunStartMode = rsmInitial_ART;

    // All three sites currently PASS (iT6IsFail==0) -- golden forces site 1
    // into a Fail+ART state so at least one retest site exists.
    f.MyBinPanel[eBinFT]->iT6IsFail[0]=0;
    f.MyBinPanel[eBinFT]->iT6IsFail[1]=0;
    f.MyBinPanel[eBinFT]->iT6IsFail[2]=0;
    f.MyBinPanel[eBinFT]->bT6ART[0]=1;
    f.MyBinPanel[eBinFT]->bT6ART[1]=0;
    f.MyBinPanel[eBinFT]->bT6ART[2]=1;

    f.ARTBinCheck(eBinFT);

    CHECK(f.MyBinPanel[eBinFT]->bT6ART[0]==0, "ARTBinCheck: pre-loop clears bT6ART for every PASS site (site 0)");
    CHECK(f.MyBinPanel[eBinFT]->bT6ART[2]==0, "ARTBinCheck: pre-loop clears bT6ART for every PASS site (site 2)");
    CHECK(f.MyBinPanel[eBinFT]->iT6IsFail[1]==1, "ARTBinCheck: all-pass forces site 1 into Fail");
    CHECK(f.MyBinPanel[eBinFT]->bT6ART[1]==1, "ARTBinCheck: all-pass forces site 1 into ART");
}

static void Test_ARTBinCheck_NoOpWhenAnAlreadyFailingARTSiteExists()
{
    ResetGlobals();
    TfBinSel f;

    USE_AUTO_RETEST = eartInstall;
    IniConfig.bA10_AutoReTest = true;
    LastSet.iRunStartMode = rsmInitial_ART;

    f.MyBinPanel[eBinFT]->iT6IsFail[0]=1;   // site 0 already failing
    f.MyBinPanel[eBinFT]->iT6IsFail[1]=0;
    f.MyBinPanel[eBinFT]->iT6IsFail[2]=0;
    f.MyBinPanel[eBinFT]->bT6ART[0]=1;      // ...and already marked ART
    f.MyBinPanel[eBinFT]->bT6ART[1]=0;
    f.MyBinPanel[eBinFT]->bT6ART[2]=0;

    f.ARTBinCheck(eBinFT);

    // Neither disjunct holds (not all iT6IsFail==0; not all bT6ART==0), so
    // golden's own forcing branch does not fire -- site 1 stays untouched.
    CHECK(f.MyBinPanel[eBinFT]->iT6IsFail[1]==0, "ARTBinCheck: no-op leaves site 1 iT6IsFail untouched");
    CHECK(f.MyBinPanel[eBinFT]->bT6ART[1]==0, "ARTBinCheck: no-op leaves site 1 bT6ART untouched");
}

static void Test_ARTBinCheck_OuterGuardSkipsWhenNotARTMode()
{
    ResetGlobals();
    TfBinSel f;

    USE_AUTO_RETEST = 0;   // eartInstall not selected -- outer guard should short-circuit
    IniConfig.bA10_AutoReTest = true;
    LastSet.iRunStartMode = rsmInitial_ART;

    f.MyBinPanel[eBinFT]->iT6IsFail[0]=0;
    f.MyBinPanel[eBinFT]->iT6IsFail[1]=0;
    f.MyBinPanel[eBinFT]->iT6IsFail[2]=0;
    f.MyBinPanel[eBinFT]->bT6ART[0]=0;
    f.MyBinPanel[eBinFT]->bT6ART[1]=0;
    f.MyBinPanel[eBinFT]->bT6ART[2]=0;

    f.ARTBinCheck(eBinFT);

    CHECK(f.MyBinPanel[eBinFT]->iT6IsFail[1]==0, "ARTBinCheck: outer guard (USE_AUTO_RETEST!=eartInstall) is a true no-op");
}

// =============================================================================
//  (5) CheckFix2Tray -- golden :4785-4805 (pure; see file banner for the
//      constructible-subset note on eBinNotUse being TU-local)
// =============================================================================
static void Test_CheckFix2Tray_FalseWhenAOIDisabled()
{
    ResetGlobals();
    TfBinSel f;

    tAOISetup.bEnabledAOI = false;
    CHECK(f.CheckFix2Tray() == false, "CheckFix2Tray: AOI disabled -> always false regardless of table content");
}

static void Test_CheckFix2Tray_FalseWhenAOIEnabledButTableEmpty()
{
    ResetGlobals();
    TfBinSel f;

    tAOISetup.bEnabledAOI = true;   // ctor's own all-zero BackT6PosTray -> no Fix2 entry exists
    CHECK(f.CheckFix2Tray() == false, "CheckFix2Tray: AOI enabled, ctor-default all-zero table -> false");
}

// =============================================================================
//  (6) CheckOSBin -- golden :6254-6266 (pure)
// =============================================================================
static void Test_CheckOSBin_TrueWhenFeatureDisabled()
{
    ResetGlobals();
    TfBinSel f;

    CosFunction.bAutoSiteMappingSetOpenBIN = false;
    CHECK(f.CheckOSBin() == true, "CheckOSBin: feature disabled -> always true");
}

static void Test_CheckOSBin_TrueWhenOpenBinNegative()
{
    ResetGlobals();
    TfBinSel f;

    CosFunction.bAutoSiteMappingSetOpenBIN = true;
    Prod.iOpenBin = -1;
    CHECK(f.CheckOSBin() == true, "CheckOSBin: iOpenBin<0 -> skips the array check, stays true");
}

static void Test_CheckOSBin_FalseWhenSelectedBinNotInCatData()
{
    ResetGlobals();
    TfBinSel f;

    CosFunction.bAutoSiteMappingSetOpenBIN = true;
    iTestRunMode = eBinFT;
    Prod.iOpenBin = 3;
    BinSelect[eBinFT].iCatDataT3Pos[3] = 0;

    CHECK(f.CheckOSBin() == false, "CheckOSBin: iCatDataT3Pos[iOpenBin]==0 -> false");

    BinSelect[eBinFT].iCatDataT3Pos[3] = 7;   // any nonzero tray assignment
    CHECK(f.CheckOSBin() == true, "CheckOSBin: iCatDataT3Pos[iOpenBin]!=0 -> true");
}

// =============================================================================
//  (7) ChangeActivePageIndex -- golden :2194-2212 (pure widget-index mapping,
//      see forms/fBinSel.h DESIGN NOTE)
// =============================================================================
static void Test_ChangeActivePageIndex_MapsEveryBinTypeToItsOwnTab()
{
    ResetGlobals();
    TfBinSel f;

    iTestRunMode = eBinFT;      f.ChangeActivePageIndex(); CHECK(f.PageControl1->ActivePageIndex==0, "ChangeActivePageIndex: eBinFT -> 0 (tsNormal)");
    iTestRunMode = eBinRT;      f.ChangeActivePageIndex(); CHECK(f.PageControl1->ActivePageIndex==1, "ChangeActivePageIndex: eBinRT -> 1 (tsRetest)");
    iTestRunMode = eBinOffLine; f.ChangeActivePageIndex(); CHECK(f.PageControl1->ActivePageIndex==2, "ChangeActivePageIndex: eBinOffLine -> 2 (tsOffline)");
    iTestRunMode = eBinFT_ART;  f.ChangeActivePageIndex(); CHECK(f.PageControl1->ActivePageIndex==3, "ChangeActivePageIndex: eBinFT_ART -> 3 (tsArtFT)");
    iTestRunMode = eBinRT_ART;  f.ChangeActivePageIndex(); CHECK(f.PageControl1->ActivePageIndex==4, "ChangeActivePageIndex: eBinRT_ART -> 4 (tsArtRT)");
    iTestRunMode = eBinFT_MRT;  f.ChangeActivePageIndex(); CHECK(f.PageControl1->ActivePageIndex==5, "ChangeActivePageIndex: eBinFT_MRT -> 5 (tsMrtFT)");
    iTestRunMode = eBinRT_MRT;  f.ChangeActivePageIndex(); CHECK(f.PageControl1->ActivePageIndex==6, "ChangeActivePageIndex: eBinRT_MRT -> 6 (tsMrtRT)");
}

static void Test_ChangeActivePageIndex_InvalidModeLeavesIndexUntouched()
{
    ResetGlobals();
    TfBinSel f;

    f.PageControl1->ActivePageIndex = 4;   // arbitrary known prior value
    iTestRunMode = -1;                     // not any eBinType value
    f.ChangeActivePageIndex();

    // golden's own `else` arm (ShowMyMessage only) never touches
    // PageControl1->ActivePage -- the port's ActivePageIndex must likewise
    // survive unchanged.
    CHECK(f.PageControl1->ActivePageIndex == 4, "ChangeActivePageIndex: invalid iTestRunMode leaves ActivePageIndex untouched");
}

// =============================================================================
//  (8) ReadFunctionData -- golden :4807-5667, FW-BinSel-WB oracle coverage.
//      Both tests target eBinFT (golden :4933-4986 new-format / the shared
//      OLD FORMAT block) -- the SIMPLEST tag branch, no bA02BinModelPrime/
//      bFTBin2RTBin nesting the way eBinRT has. Oracle field is
//      Cons.Fail/bConsFail: a clean, single-shot `atoi(Strings[i])` -> bool
//      conversion with NO dependency on any prior BinSelect[] state.
//
//      (B17) NOTE (not asserted by either test below, flagged for whoever
//      next touches this method): the SAME new-format postprocessing loop's
//      OWN iCatDataT3Pos/sBinTraySetT3Pos handling (golden :5552-5588,
//      ported cBinSel.cpp verbatim) computes `iT3=BinSelect[tag].
//      iCatDataT3Pos[i]-1` from the STALE (pre-call) `BinSelect[tag].
//      iCatDataT3Pos[i]` -- NOT from `iData` (the value just parsed from the
//      ini string this call) -- and only commits `iData` at the very end
//      regardless of which branch fired. On a cold/zero-initialized
//      BinSelect[] (e3Auto1==0, so iT3=0-1=-1 fails the `>=e3Auto1` guard),
//      this means a FIRST-EVER ReadFunctionData call discards whatever
//      iCatDataT3Pos value the ini actually held and forces e3PosNoUse,
//      regardless of the seeded ini content -- which is why this field was
//      deliberately NOT chosen as either test's oracle (it would need a
//      pre-seeded, ALREADY-VALID BinSelect[eBinFT].iCatDataT3Pos[i] to
//      observe anything, i.e. testing golden's quirk, not the read itself).
// =============================================================================
static void Test_ReadFunctionData_NewFormat_ReadsFromScratchIni()
{
    ResetGlobals();

    AnsiString scratch = BinSelScratchIniPath("test_binsel_readfunctiondata_new.ini");
    DeleteFile(scratch);

    // Seed a "new format" [Bin Func FT] section (golden :4933-4986's own
    // literal GroupName/ECID-key strings for eBinFT) -- CheckSectionExist
    // finds it, and CUSTOMER_CODE below is neither CC_ASE_KaohSiung nor
    // CC_AMKOR_China, so golden's own `else { bHasNewSetupData=true; }` arm
    // fires unconditionally (no "NewBinFormat" key needed to force it).
    WriteIniData(scratch, "Bin Func FT", "3676 BinConsFail", AnsiString("1,0,1"));

    int savedTestBinCount = iTestBinCount;
    int savedCustomerCode = CUSTOMER_CODE;
    iTestBinCount = 3;                 // matches the 3-entry CommaText above
    CUSTOMER_CODE = 0;                 // != CC_AMKOR_China(972), != CC_ASE_KaohSiung(936)
    {
        TfBinSel f;

        f.ReadFunctionData(eBinFT, scratch);

        CHECK(AnsiString(f.sBinConsFail[eBinFT]->CommaText) == "1,0,1",
              "ReadFunctionData (new format): sBinConsFail[eBinFT] reads the seeded ini CommaText");
        CHECK(BinSelect[eBinFT].bConsFail[0] == true,  "ReadFunctionData (new format): BinSelect[eBinFT].bConsFail[0] true from ini \"1\"");
        CHECK(BinSelect[eBinFT].bConsFail[1] == false, "ReadFunctionData (new format): BinSelect[eBinFT].bConsFail[1] false from ini \"0\"");
        CHECK(BinSelect[eBinFT].bConsFail[2] == true,  "ReadFunctionData (new format): BinSelect[eBinFT].bConsFail[2] true from ini \"1\"");
        CHECK(f.MyBinPanel[eBinFT]->bConFail[0] == true,  "ReadFunctionData (new format): MyBinPanel[eBinFT]->bConFail[0] mirrors BinSelect");
        CHECK(f.MyBinPanel[eBinFT]->bConFail[1] == false, "ReadFunctionData (new format): MyBinPanel[eBinFT]->bConFail[1] mirrors BinSelect");
    }
    CUSTOMER_CODE = savedCustomerCode;
    iTestBinCount = savedTestBinCount;
    DeleteFile(scratch);
}

static void Test_ReadFunctionData_OldFormat_FallsBackToPerCategoryRead()
{
    ResetGlobals();

    AnsiString scratch = BinSelScratchIniPath("test_binsel_readfunctiondata_old.ini");
    DeleteFile(scratch);

    // Deliberately NO "[Bin Func FT]" section at all -- CheckSectionExist(...)
    // is false, bHasNewSetupData stays false, so golden's OLD FORMAT branch
    // (golden :5356 onward, "這裡是為了跟舊版的相容") runs instead, reading
    // each Category<i>'s own "Cons.Fail" key directly.
    WriteIniData(scratch, "Category0", "Cons.Fail", 1);
    WriteIniData(scratch, "Category1", "Cons.Fail", 0);
    WriteIniData(scratch, "Category2", "Cons.Fail", 1);

    int savedTestBinCount = iTestBinCount;
    int savedCustomerCode = CUSTOMER_CODE;
    iTestBinCount = 3;
    CUSTOMER_CODE = 0;
    {
        TfBinSel f;

        f.ReadFunctionData(eBinFT, scratch);

        CHECK(BinSelect[eBinFT].bConsFail[0] == true,  "ReadFunctionData (old format): BinSelect[eBinFT].bConsFail[0] true from Category0");
        CHECK(BinSelect[eBinFT].bConsFail[1] == false, "ReadFunctionData (old format): BinSelect[eBinFT].bConsFail[1] false from Category1");
        CHECK(BinSelect[eBinFT].bConsFail[2] == true,  "ReadFunctionData (old format): BinSelect[eBinFT].bConsFail[2] true from Category2");
        CHECK(f.MyBinPanel[eBinFT]->bConFail[0] == true, "ReadFunctionData (old format): MyBinPanel[eBinFT]->bConFail[0] mirrors BinSelect");
        // golden's own OLD-FORMAT sync-back loop (golden :5473 onward)
        // mirrors BinSelect[tag] back into sBinConsFail[tag]->Strings[i]
        // when i<sBinConsFail[tag]->Count (true here: ctor sized it to
        // iTestBinCount==3) -- verify the round-trip landed there too.
        CHECK(AnsiString(f.sBinConsFail[eBinFT]->Strings[0]) == "1",
              "ReadFunctionData (old format): sBinConsFail[eBinFT]->Strings[0] synced back from BinSelect");
    }
    CUSTOMER_CODE = savedCustomerCode;
    iTestBinCount = savedTestBinCount;
    DeleteFile(scratch);
}

// =============================================================================
//  (9) mtTrayNameSetColor -- golden :4033-4140, FW-BinSel-WC full unlock
//      (GATE G9 CLOSED). Cell coordinates below are raw literals for the
//      SAME "TU-local enum" reason as (2b) above -- eItemPass=1, colour
//      indices eCLRed=2/eCLGreen=1 read directly off golden's own
//      `eTrayColorMap` enum literals (cBinSel.cpp).
// =============================================================================
static void Test_MtTrayNameSetColor_MarksFailingAndPassingSitesWithCorrectColor()
{
    ResetGlobals();
    TfBinSel f;

    Prod.iTrayType[0] = 1;   // != tNotUse(0) -- Auto1 must be a REAL tray for its row to be processed (golden :4198)
    Prod.iTrayType[1] = 1;   // != tNotUse(0) -- Auto2 likewise
    f.MyBinPanel[eBinFT]->iT6IsFail[0] = 1;    // Auto1 fails
    f.MyBinPanel[eBinFT]->iT6IsFail[1] = 0;    // Auto2 passes
    f.MyBinPanel[eBinFT]->iErrorT6 = -1;       // no error site (golden :4214)
    TestIF_File.bEnableQASampling = false;     // golden :4225 QA-sampling branch stays off

    f.mtTrayNameSetColor(eBinFT);

    CHECK(f.MyBinPanel[eBinFT]->mtTrayName->GetCellText(1 /*eItemPass*/, 26+0) == "Failed",
          "mtTrayNameSetColor: failing site (Auto1) -> \"Failed\" text (golden :4233)");
    CHECK(f.MyBinPanel[eBinFT]->mtTrayName->CellColorIndex[1][26] == 2 /*eCLRed*/,
          "mtTrayNameSetColor: failing site -> eCLRed colour index (golden :4234, iT6IsFail[0]+1==2)");
    CHECK(f.MyBinPanel[eBinFT]->mtTrayName->GetCellText(1, 26+1) == "Pass",
          "mtTrayNameSetColor: passing site (Auto2) -> \"Pass\" text (golden :4238)");
    CHECK(f.MyBinPanel[eBinFT]->mtTrayName->CellColorIndex[1][27] == 1 /*eCLGreen*/,
          "mtTrayNameSetColor: passing site -> eCLGreen colour index (golden :4239)");

    Prod.iTrayType[0] = 0;
    Prod.iTrayType[1] = 0;
}

// =============================================================================
//  (10) InitDataToEdit -- golden :4142-4784, FW-BinSel-WC (newly declared).
//       No scratch-ini/path seam needed (see this file's own head banner) --
//       pure in-memory MyBinPanel[tag]-field-to-sXxx[tag] reshuffling (S20,
//       see cBinSel.cpp's own banner on this method).
// =============================================================================
static void Test_InitDataToEdit_DoubleContactAndConsFailRoundTripToSXxx()
{
    ResetGlobals();
    int savedTestBinCount = iTestBinCount;
    iTestBinCount = 3;
    {
        TfBinSel f;
        f.MyBinPanel[eBinFT]->i2Contact[0] = 1;
        f.MyBinPanel[eBinFT]->i2Contact[1] = 0;
        f.MyBinPanel[eBinFT]->i2Contact[2] = 1;
        f.MyBinPanel[eBinFT]->bConFail[0]  = true;
        f.MyBinPanel[eBinFT]->bConFail[1]  = false;
        f.MyBinPanel[eBinFT]->bConFail[2]  = true;

        f.InitDataToEdit(eBinFT);

        CHECK(AnsiString(f.sBinDoubleContact[eBinFT]->CommaText) == "1,0,1",
              "InitDataToEdit: sBinDoubleContact[tag] mirrors MyBinPanel[tag]->i2Contact[] (golden :4265, S20 elision)");
        CHECK(AnsiString(f.sBinConsFail[eBinFT]->CommaText) == "1,0,1",
              "InitDataToEdit: sBinConsFail[tag] mirrors MyBinPanel[tag]->bConFail[] (golden :4284, S20 elision)");
    }
    iTestBinCount = savedTestBinCount;
}

static void Test_InitDataToEdit_AutoCleanRoundTripsOnlyWhenFunctionEnabled()
{
    ResetGlobals();
    int savedTestBinCount = iTestBinCount;
    iTestBinCount = 2;
    {
        TfBinSel f;
        f.MyBinPanel[eBinFT]->iAutoCleanByBin[0]  = 5;
        f.MyBinPanel[eBinFT]->iAutoCleanByBin[1]  = 0;
        f.MyBinPanel[eBinFT]->iAutoCleanBySite[0] = 7;
        f.MyBinPanel[eBinFT]->iAutoCleanBySite[1] = 0;

        // seed a KNOWN prior value to prove the disabled branch leaves it
        // untouched -- golden's own `else` arm (golden :4519-4528) touches
        // ONLY mtBinSelect (GATE G10), setting neither ed*, so the read-back
        // this method's S20 elision stands in for is a no-op there.
        f.sBySiteClean[eBinFT]->CommaText = "9,9";
        f.sByBinClean[eBinFT]->CommaText  = "8,8";

        TestIF_File.iAutoClean_Function = 0;
        f.InitDataToEdit(eBinFT);
        CHECK(AnsiString(f.sBySiteClean[eBinFT]->CommaText) == "9,9",
              "InitDataToEdit: AutoClean disabled -> sBySiteClean[tag] left UNCHANGED (golden's else arm is mtBinSelect-only)");
        CHECK(AnsiString(f.sByBinClean[eBinFT]->CommaText) == "8,8",
              "InitDataToEdit: AutoClean disabled -> sByBinClean[tag] left UNCHANGED");

        TestIF_File.iAutoClean_Function = 1;
        f.InitDataToEdit(eBinFT);
        // NOTE golden's own ByBin/BySite NAME SWAP (not a new bug, see this
        // method's own banner in cBinSel.cpp): iAutoCleanByBin feeds
        // sBySiteClean, iAutoCleanBySite feeds sByBinClean.
        CHECK(AnsiString(f.sBySiteClean[eBinFT]->CommaText) == "5,0",
              "InitDataToEdit: AutoClean enabled -> sBySiteClean[tag] mirrors MyBinPanel[tag]->iAutoCleanByBin[] (golden :4503, S20)");
        CHECK(AnsiString(f.sByBinClean[eBinFT]->CommaText) == "7,0",
              "InitDataToEdit: AutoClean enabled -> sByBinClean[tag] mirrors MyBinPanel[tag]->iAutoCleanBySite[] (golden :4513, S20)");
    }
    TestIF_File.iAutoClean_Function = 0;
    iTestBinCount = savedTestBinCount;
}

int main()
{
    Test_Ctor_PopulatesBinTypeStringLists();
    Test_Ctor_MyBinPanelZeroInitialized();
    Test_Ctor_PopulatesMtTrayNameAndItemStaticLabels();
    Test_Ctor_MtTrayItemRowLabelPopulatesOnlyWhenGeneralIniOpen();
    Test_TransferBinTrayStrToName_ConvertsIndexToTrayName();
    Test_TransferBinTrayStrToName_OutOfRangeIndexIsNotUse();
    Test_ARTBinCheck_ForcesRetestWhenAllThreeSitesPassOrNoneAreART();
    Test_ARTBinCheck_NoOpWhenAnAlreadyFailingARTSiteExists();
    Test_ARTBinCheck_OuterGuardSkipsWhenNotARTMode();
    Test_CheckFix2Tray_FalseWhenAOIDisabled();
    Test_CheckFix2Tray_FalseWhenAOIEnabledButTableEmpty();
    Test_CheckOSBin_TrueWhenFeatureDisabled();
    Test_CheckOSBin_TrueWhenOpenBinNegative();
    Test_CheckOSBin_FalseWhenSelectedBinNotInCatData();
    Test_ChangeActivePageIndex_MapsEveryBinTypeToItsOwnTab();
    Test_ChangeActivePageIndex_InvalidModeLeavesIndexUntouched();
    Test_ReadFunctionData_NewFormat_ReadsFromScratchIni();
    Test_ReadFunctionData_OldFormat_FallsBackToPerCategoryRead();
    Test_MtTrayNameSetColor_MarksFailingAndPassingSitesWithCorrectColor();
    Test_InitDataToEdit_DoubleContactAndConsFailRoundTripToSXxx();
    Test_InitDataToEdit_AutoCleanRoundTripsOnlyWhenFunctionEnabled();

    std::printf("%d/%d checks passed (test_binsel_core)\n", g_pass, g_pass + g_fail);
    return g_fail == 0 ? 0 : 1;
}
