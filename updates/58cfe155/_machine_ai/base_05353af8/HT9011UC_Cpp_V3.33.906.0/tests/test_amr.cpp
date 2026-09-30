// =============================================================================
//  test_amr.cpp  --  W5 VERIFY: TeraPower AMR bridge (Automation/AMR.cpp) +
//                     the 3 pure AGV AMR-mode predicates (AGV_predicates.cpp)
//
//  Translation wave: W5 (Automation network unit)
//  Author: AI(W5-AMR-Translate) 20260710
//
//  PURPOSE
//  -------
//  Drives the ACTUAL translated TTeraPowerAMR (global `AMR` instance) + the
//  3 free-function AGV predicates over a KNOWN global-state baseline (no
//  hardware) and asserts outputs against values hand-derived from the golden
//  BCB6 formulas (golden file:line cited per case, same style as
//  test_w6_3_catchtray_core.cpp / test_cContact.cpp).
//
//  EQUIVALENCE NOTE: no Borland binary exists, so "equivalence" here == clean
//  g++ compile/link + the translated code producing outputs that match values
//  hand-derived from the golden source, over an explicitly-set global-state
//  baseline (this TU owns global mutable state -- CosFunction/IniConfig/
//  TestIF_File/BinSelect/LastSet/MOT[]/fSCKART -- so each PART resets exactly
//  the fields it reads before asserting, to stay independent of PART order).
//
//  BUILD NOTE (expected, not a bug in this test): this TU will not compile
//  until the integrate agent adds the 8 flagged fields/methods this unit's
//  translate report lists (6 LAST_GENERAL_SET fields in canary_support.h +
//  TfLotInfo::RefreshAMR() + TfSCKART::iCurrent93KARTStep in FormsFacade.h).
//  AMR.cpp's own syntax was independently verified clean via a scratch-local
//  patched copy of those two shared headers (see translate report) -- the
//  compile-only failure today is exactly the 8 documented gaps, nothing else.
// =============================================================================
#include "Automation/AMR.h"
#include "Automation/AGV_predicates.h"
#include "cprod.h"                 // IniConfig / CosFunction / TestIF_File / BinSelect / MachineType enums
#include "cmydef.h"                 // iAutoTrayCount[] / iTrayFeed / iTestRunMode / iTestBinCount / USE_COVER_TRAYID / CUSTOMER_CODE
#include "Motor/mymotor.h"          // MOT[]
#include "FormsFacade.h"            // fSCKART / fLotInfo
#include "canary_support.h"         // LastSet
#include <cstdio>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; }\
    } while (0)

// ---------------------------------------------------------------------------
//  Shared-state reset helpers -- each PART calls the relevant one(s) first so
//  PARTS are independent of execution order (global mutable state).
// ---------------------------------------------------------------------------
static void ResetAMRLastSet()
{
    for (int i = 0; i < 9; i++) LastSet.iAMRTrayConut[i] = 0;
    LastSet.iAMRTrayLoaderTotal   = 0;
    LastSet.bAMRTrayFeedWait      = false;
    LastSet.bAMRLoaderLast        = false;
    LastSet.bAMRRequestSupplyTray = false;
    for (int i = 0; i < 256; i++) LastSet.iUnloaderTrayCount_ART[i] = 0;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 260; j++)
            LastSet.iBinData32[i][j] = 0;
}

static void ResetIniConfigForAMR()
{
    IniConfig.bA60EnableAMR         = true;
    IniConfig.iA60LoaderQtyAtOneTime = 5;
    for (int i = 0; i < 9; i++) IniConfig.iA60NotifyQty[i] = 3;
    IniConfig.bA10_AutoReTest       = false;
}

static void ResetBinSelectForAMR()
{
    iTestRunMode = 0;
    iTestBinCount = 4;                      // bins 0..3
    SYSTEM_BIN_SELECT &bs = BinSelect[0];
    // NOTE: iCatDataT3Pos[] is sized TEST_MAX_BIN(256, MachineType.h:414); bAutoRetest[]/
    // bCateR[]/iStackDefFailCate[] are sized eTrayCount(33, MachineType.h:1104) -- do NOT
    // share one loop bound across both (that would write past the smaller arrays).
    for (int i = 0; i < TEST_MAX_BIN; i++)
        bs.iCatDataT3Pos[i] = 0;             // 0 == "no category" (golden: continue)
    for (int i = 0; i < eTrayCount; i++)
    {
        bs.bAutoRetest[i]        = false;
        bs.bCateR[i]             = false;
        bs.iStackDefFailCate[i]  = 0;
    }
    bs.IfErrorT3 = -1;                        // no bin maps to the Error position by default
}

int main()
{
    printf("==== W5 AMR + AGV-predicates verify ====\n");

    // =========================================================================
    //  PART A -- AGV_predicates (golden Automation/AGV.cpp:1315-1348)
    // =========================================================================
    printf("[A] AGV_predicates (golden AGV.cpp:1315-1348)\n");
    {
        // A1: IsSPIL_AMR -- ALL three conditions must hold (golden :1317-1322).
        USE_COVER_TRAYID = tCID_Keyence;
        IniConfig.bA65_BundleIDList = true;
        IniConfig.bSPILFunction     = true;
        CHECK(AGV_IsSPIL_AMR() == true, "IsSPIL_AMR: Keyence+BundleID+SPIL -> true (golden :1317)");

        IniConfig.bSPILFunction = false;
        CHECK(AGV_IsSPIL_AMR() == false, "IsSPIL_AMR: SPILFunction=false -> false (golden :1322 no-else path)");

        USE_COVER_TRAYID = tCID_NFC;
        IniConfig.bSPILFunction = true;
        CHECK(AGV_IsSPIL_AMR() == false, "IsSPIL_AMR: wrong reader (NFC, not Keyence) -> false");

        // A2: IsATK_AMR -- NFC + BundleID + CC_AMKOR_Korea (golden :1329-1334).
        USE_COVER_TRAYID = tCID_NFC;
        IniConfig.bA65_BundleIDList = true;
        CUSTOMER_CODE = CC_AMKOR_Korea;
        CHECK(AGV_IsATK_AMR() == true, "IsATK_AMR: NFC+BundleID+AMKOR_Korea -> true (golden :1329)");

        CUSTOMER_CODE = 0;   // any non-AMKOR customer code
        CHECK(AGV_IsATK_AMR() == false, "IsATK_AMR: wrong customer -> false");

        USE_COVER_TRAYID = tCID_Keyence;
        CUSTOMER_CODE = CC_AMKOR_Korea;
        CHECK(AGV_IsATK_AMR() == false, "IsATK_AMR: wrong reader (Keyence, not NFC) -> false");

        // A3: Use_AMR -- ANY reader configured + BundleID (golden :1340-1345).
        USE_COVER_TRAYID = tCID_Keyence;
        IniConfig.bA65_BundleIDList = true;
        CHECK(AGV_Use_AMR() == true, "Use_AMR: Keyence+BundleID -> true (golden :1340)");

        USE_COVER_TRAYID = tCID_NFC;
        CHECK(AGV_Use_AMR() == true, "Use_AMR: NFC+BundleID -> true (any reader qualifies)");

        USE_COVER_TRAYID = tCIDNotUse;
        CHECK(AGV_Use_AMR() == false, "Use_AMR: tCIDNotUse -> false (golden :1340 guard)");

        USE_COVER_TRAYID = tCID_Keyence;
        IniConfig.bA65_BundleIDList = false;
        CHECK(AGV_Use_AMR() == false, "Use_AMR: BundleID disabled -> false");
    }

    // =========================================================================
    //  PART B -- TTeraPowerAMR (golden Automation/AMR.cpp)
    // =========================================================================
    printf("[B] TTeraPowerAMR (golden AMR.cpp)\n");

    // B0: master gate -- CosFunction.bEnableHandlerResultServer==false short-
    // circuits every counting method to a safe default (false / no-op).
    printf("  [B0] master gate: bEnableHandlerResultServer==false\n");
    {
        CosFunction.bEnableHandlerResultServer = false;
        IniConfig.bA60EnableAMR = true;
        ResetAMRLastSet();
        LastSet.bAMRRequestSupplyTray = true;   // would be true if the gate were open
        LastSet.iAMRTrayConut[0] = 5;            // would make CheckTrayFeed() true if the gate were open
        CHECK(AMR.WaitAMRSupplyTray() == false, "WaitAMRSupplyTray: gate closed -> false (golden :76-77)");
        CHECK(AMR.CheckLoaderCount() == false,  "CheckLoaderCount: gate closed -> false (golden :86-89)");
        CHECK(AMR.CheckUnloaderCount(0) == false, "CheckUnloaderCount: gate closed -> false (golden :126-127)");
        CHECK(AMR.CheckTrayFeed() == false, "CheckTrayFeed: gate closed -> false, even with nonzero iAMRTrayConut[0] (golden :182-183)");
        CHECK(AMR.NeedAMRTransport() == false, "NeedAMRTransport: gate closed -> false (golden :451-454)");
        AMR.Initial();
        CHECK(LastSet.bAMRRequestSupplyTray == true, "Initial: gate closed -> no-op, field left UNCHANGED (golden :23-24 early return)");
    }

    // Open the master gate for the rest of the PARTs.
    CosFunction.bEnableHandlerResultServer = true;
    ResetIniConfigForAMR();
    TestIF_File.bSCKART_EnableART = false;
    iTrayFeed = 0;

    // AI(W906-FW3-LotInfo-WBfix) 20260819: golden runtime precondition, replayed
    // as fixture setup -- FormShow (golden uLotInfo.cpp:1027) always runs
    // ShowAMRCategoryBin(true) before any refresh, sizing StrGrdCategory to
    // iTestBinCount+2 rows. RefreshAMR was a no-op stub when this test was
    // written; the LotInfo Wave B translation made it real, so Initial()'s
    // refresh now reaches the grid and, without this, runs off the .dfm row
    // count exactly as golden would if FormShow had never fired.
    fLotInfo->ShowAMRCategoryBin(true);

    // B1: Initial() (golden :22-37)
    printf("  [B1] Initial()\n");
    {
        ResetAMRLastSet();
        LastSet.iAMRTrayConut[0] = 999;   // will be overwritten
        LastSet.bAMRRequestSupplyTray = true;
        AMR.Initial();
        CHECK(LastSet.iAMRTrayConut[0] == IniConfig.iA60LoaderQtyAtOneTime,
              "Initial: iAMRTrayConut[0] <- iA60LoaderQtyAtOneTime (golden :24)");
        bool restZero = true;
        for (int i = 1; i <= 5; i++) if (LastSet.iAMRTrayConut[i] != 0) restZero = false;
        CHECK(restZero, "Initial: iAMRTrayConut[1..5] == 0 (golden :25-29)");
        CHECK(LastSet.iAMRTrayLoaderTotal == 0, "Initial: iAMRTrayLoaderTotal == 0 (golden :30)");
        CHECK(LastSet.bAMRTrayFeedWait == false, "Initial: bAMRTrayFeedWait == false (golden :31)");
        CHECK(LastSet.bAMRLoaderLast == false, "Initial: bAMRLoaderLast == false (golden :32)");
        CHECK(LastSet.bAMRRequestSupplyTray == false, "Initial: bAMRRequestSupplyTray cleared (golden :33, Sam 20250312)");
    }

    // B2: LoaderInCount() / LoaderCarryIn() (golden :39-48 / :159-163)
    printf("  [B2] LoaderInCount / LoaderCarryIn\n");
    {
        ResetAMRLastSet();
        LastSet.iAMRTrayConut[0] = 10;
        LastSet.iAMRTrayLoaderTotal = 2;
        AMR.LoaderInCount();
        CHECK(LastSet.iAMRTrayConut[0] == 9, "LoaderInCount: iAMRTrayConut[0]-- (golden :44)");
        CHECK(LastSet.iAMRTrayLoaderTotal == 3, "LoaderInCount: iAMRTrayLoaderTotal++ (golden :45)");

        IniConfig.bA60EnableAMR = false;
        AMR.LoaderInCount();
        CHECK(LastSet.iAMRTrayConut[0] == 9, "LoaderInCount: bA60EnableAMR==false -> no-op (golden :41-42)");
        IniConfig.bA60EnableAMR = true;

        AMR.LoaderCarryIn();
        CHECK(LastSet.iAMRTrayConut[0] == 9 + IniConfig.iA60LoaderQtyAtOneTime,
              "LoaderCarryIn: iAMRTrayConut[0] += iA60LoaderQtyAtOneTime (golden :161)");
    }

    // B3: UnloaderOutCount() / UnloaderCarryOut() (golden :50-72 / :165-178)
    printf("  [B3] UnloaderOutCount / UnloaderCarryOut\n");
    {
        ResetAMRLastSet();
        for (int i = 0; i < 3; i++) iAutoTrayCount[i] = 0;

        AMR.UnloaderOutCount(0);
        CHECK(LastSet.iAMRTrayConut[3] == 1, "UnloaderOutCount(0): iAMRTrayConut[iAuto+3]++ (golden :55-56, iTrackPos=3)");

        // ART branch: IsAutoReTestTray(0) requires bA10_AutoReTest + bSCKART_EnableART
        // + BinSelect[iTestRunMode].bAutoRetest[0] all true (golden :415-424).
        ResetAMRLastSet();  // re-zero counters before configuring the ART scenario
        IniConfig.bA10_AutoReTest = true;
        TestIF_File.bSCKART_EnableART = true;
        BinSelect[iTestRunMode].bAutoRetest[0] = true;
        LastSet.iUnloaderTrayCount_ART[0] = 5;
        iAutoTrayCount[0] = 5;
        AMR.UnloaderOutCount(0);   // iAMRTrayConut[3] 0->1 (==1) AND IsAutoReTestTray(0)==true
        CHECK(LastSet.iAMRTrayConut[3] == 1, "UnloaderOutCount(0) ART: iAMRTrayConut[3]++ -> 1 (golden :56)");
        CHECK(LastSet.iUnloaderTrayCount_ART[0] == 4, "UnloaderOutCount(0) ART: iUnloaderTrayCount_ART[0]-- (golden :58)");
        CHECK(iAutoTrayCount[0] == 4, "UnloaderOutCount(0) ART: iAutoTrayCount[0]-- (golden :59)");
        // reset ART config for the rest of the file
        IniConfig.bA10_AutoReTest = false;
        TestIF_File.bSCKART_EnableART = false;
        BinSelect[iTestRunMode].bAutoRetest[0] = false;

        // invalid iAuto -> ShowMyMessage("UnloaderOutCount error","") + no crash, no field touched
        ResetAMRLastSet();
        AMR.UnloaderOutCount(9);
        CHECK(LastSet.iAMRTrayConut[3] == 0, "UnloaderOutCount(9) invalid: no field mutated (golden :66-69 else)");

        ResetAMRLastSet();
        LastSet.iAMRTrayConut[4] = 7;
        iAutoTrayCount[1] = 3;
        AMR.UnloaderCarryOut(1);
        CHECK(LastSet.iAMRTrayConut[4] == 0, "UnloaderCarryOut(1): iAMRTrayConut[iAuto+3]=0 (golden :169)");
        CHECK(iAutoTrayCount[1] == 0, "UnloaderCarryOut(1): iAutoTrayCount[1]=0 (golden :170, Sam 20250430)");

        ResetAMRLastSet();
        AMR.UnloaderCarryOut(9);   // invalid -> ShowMyMessage, no crash (golden :175-177)
        CHECK(true, "UnloaderCarryOut(9) invalid: no crash (golden :175-177 else)");
    }

    // B4: WaitAMRSupplyTray() (golden :74-82)
    printf("  [B4] WaitAMRSupplyTray\n");
    {
        ResetAMRLastSet();
        LastSet.bAMRRequestSupplyTray = true;
        CHECK(AMR.WaitAMRSupplyTray() == true, "WaitAMRSupplyTray: bAMRRequestSupplyTray==true -> true (golden :79)");
        LastSet.bAMRRequestSupplyTray = false;
        CHECK(AMR.WaitAMRSupplyTray() == false, "WaitAMRSupplyTray: false -> false (golden :81)");
    }

    // B5: CheckLoaderCount() plain (non-ART) branch (golden :84-123, plain path :113-122)
    printf("  [B5] CheckLoaderCount (plain branch)\n");
    {
        IniConfig.bA10_AutoReTest = false;   // forces the plain (non-ART) branch
        ResetAMRLastSet();
        LastSet.bAMRLoaderLast = false;
        LastSet.iAMRTrayConut[0] = IniConfig.iA60NotifyQty[0];       // ==3, boundary "<=" -> true
        CHECK(AMR.CheckLoaderCount() == true, "CheckLoaderCount: iAMRTrayConut[0]<=iA60NotifyQty[0] (boundary) -> true (golden :117)");
        LastSet.iAMRTrayConut[0] = IniConfig.iA60NotifyQty[0] + 1;   // strictly above -> false
        CHECK(AMR.CheckLoaderCount() == false, "CheckLoaderCount: iAMRTrayConut[0] > threshold -> false (golden :121)");

        LastSet.bAMRLoaderLast = true;       // overrides everything -> false (golden :96-99)
        CHECK(AMR.CheckLoaderCount() == false, "CheckLoaderCount: bAMRLoaderLast==true -> false regardless (golden :96-99)");
        LastSet.bAMRLoaderLast = false;
    }

    // B6: CheckUnloaderCount() plain branch + iTrayFeed guard (golden :124-157)
    printf("  [B6] CheckUnloaderCount\n");
    {
        IniConfig.bA10_AutoReTest = false;
        ResetAMRLastSet();
        iTrayFeed = 1;
        CHECK(AMR.CheckUnloaderCount(0) == false, "CheckUnloaderCount: iTrayFeed==1 -> false (golden :129-130)");
        iTrayFeed = 0;

        LastSet.iAMRTrayConut[3] = IniConfig.iA60NotifyQty[3];       // ==3, boundary ">=" -> true
        CHECK(AMR.CheckUnloaderCount(0) == true, "CheckUnloaderCount(0): iAMRTrayConut[3]>=threshold (boundary) -> true (golden :151)");
        LastSet.iAMRTrayConut[3] = IniConfig.iA60NotifyQty[3] - 1;   // below -> false
        CHECK(AMR.CheckUnloaderCount(0) == false, "CheckUnloaderCount(0): below threshold -> false (golden :153)");

        CHECK(AMR.CheckUnloaderCount(9) == false, "CheckUnloaderCount(9) invalid iAuto -> false + ShowMyMessage (golden :155-157)");
    }

    // B7: CheckTrayFeed() (golden :180-198)
    printf("  [B7] CheckTrayFeed\n");
    {
        ResetAMRLastSet();
        CHECK(AMR.CheckTrayFeed() == false, "CheckTrayFeed: all-zero -> false (golden :196)");
        LastSet.iAMRTrayConut[2] = 1;
        CHECK(AMR.CheckTrayFeed() == true, "CheckTrayFeed: any nonzero counter -> true (golden :188)");
        ResetAMRLastSet();
        LastSet.iAMRTrayLoaderTotal = 1;
        CHECK(AMR.CheckTrayFeed() == true, "CheckTrayFeed: iAMRTrayLoaderTotal!=0 -> true (golden :192)");
    }

    // B8: GetTrackBinData / GetTrackBinDataCateR / GetNormalFailBin / ClearTrackBinData
    // (golden :201-256 / :259-317 / :319-347 / :349-373)
    printf("  [B8] Bin-data helpers (BinSelect[0], iTestBinCount=4)\n");
    {
        ResetAMRLastSet();
        ResetBinSelectForAMR();
        SYSTEM_BIN_SELECT &bs = BinSelect[0];
        // bin0 -> Auto1 (ePosAuto1==1), bin1 -> Auto2 (==2), bin2 unmapped, bin3 -> Error pos
        bs.iCatDataT3Pos[0] = ePosAuto1;   // ==1
        bs.iCatDataT3Pos[1] = ePosAuto2;   // ==2
        bs.iCatDataT3Pos[2] = 0;           // unmapped
        bs.iCatDataT3Pos[3] = 0;
        bs.IfErrorT3 = 0;                  // iErrPos+1==1==iAutoPos for iCate==0 (Auto1)
        LastSet.iBinData32[2][0] = 100;    // bin0 count
        LastSet.iBinData32[2][4] = 7;      // iTestBinCount==4 -> the Error slot

        AnsiString r0 = AMR.GetTrackBinData(0);   // iCate==0 -> Auto1
        CHECK(r0 == "Auto1=HBin0=100.HBinE=7.", "GetTrackBinData(0): bin0 match + ErrorT3 match (golden :201-256)");

        AnsiString r1 = AMR.GetTrackBinData(1);   // iCate==1 -> Auto2; bin1 IS mapped (pos==2)
        CHECK(r1 == "Auto2=HBin1=0.", "GetTrackBinData(1): bin1 matches Auto2, count 0, no Error match (golden :201-256)");

        AnsiString r2 = AMR.GetTrackBinData(2);   // iCate==2 -> Auto3 (pos==3); no bin mapped, no Error match
        CHECK(r2 == "Auto3=NA=NA", "GetTrackBinData(2): no bin mapped to Auto3 -> \"Auto3=NA=NA\" (golden :247-250)");

        // GetNormalFailBin: bin0 pos==ePosAuto1, bAutoRetest[0]=false, bCateR[0]=false,
        // iStackDefFailCate[0]==1 -> included.
        bs.iStackDefFailCate[0] = 1;
        AnsiString nf = AMR.GetNormalFailBin();
        CHECK(nf == "0", "GetNormalFailBin: only bin0 qualifies -> \"0\" (golden :319-347)");

        // Mark bin0 as CateR -> now excluded from GetNormalFailBin.
        bs.bCateR[0] = true;
        AnsiString nf2 = AMR.GetNormalFailBin();
        CHECK(nf2 == "", "GetNormalFailBin: bCateR[0]==true excludes bin0 -> \"\" (golden :323-324 guard)");
        bs.bCateR[0] = false;

        // ClearTrackBinData(0): clears bin0's iBinData32[2][*] slot + the Error slot
        // (iAutoPos==1 matches IfErrorT3+1==1).
        AMR.ClearTrackBinData(0);
        CHECK(LastSet.iBinData32[2][0] == 0, "ClearTrackBinData(0): bin0 slot cleared (golden :356-361)");
        CHECK(LastSet.iBinData32[2][4] == 0, "ClearTrackBinData(0): Error slot cleared (golden :364-367)");

        AMR.ClearTrackBinData(9);   // invalid -> ShowMyMessage, no crash
        CHECK(true, "ClearTrackBinData(9) invalid: no crash (golden :369-372 else)");
    }

    // B9: IsAutoReTestTray() (golden :414-427)
    printf("  [B9] IsAutoReTestTray\n");
    {
        ResetBinSelectForAMR();
        IniConfig.bA10_AutoReTest = false;
        CHECK(AMR.IsAutoReTestTray(0) == false, "IsAutoReTestTray: bA10_AutoReTest==false -> false (golden :416)");

        IniConfig.bA10_AutoReTest = true;
        TestIF_File.bSCKART_EnableART = true;
        BinSelect[iTestRunMode].bAutoRetest[0] = true;
        CHECK(AMR.IsAutoReTestTray(0) == true, "IsAutoReTestTray(0): ART enabled + bAutoRetest[0] -> true (golden :420-423)");
        CHECK(AMR.IsAutoReTestTray(1) == false, "IsAutoReTestTray(1): bAutoRetest[1]==false -> false");
        BinSelect[iTestRunMode].bAutoRetest[0] = false;
        IniConfig.bA10_AutoReTest = false;
        TestIF_File.bSCKART_EnableART = false;
    }

    // B10: ARTReset() (golden :429-447)
    printf("  [B10] ARTReset\n");
    {
        ResetAMRLastSet();
        ResetBinSelectForAMR();
        IniConfig.bA10_AutoReTest = true;
        TestIF_File.bSCKART_EnableART = true;
        BinSelect[iTestRunMode].bAutoRetest[1] = true;   // Auto2 (iAuto==1) is an ART track
        LastSet.bAMRLoaderLast = true;
        LastSet.iAMRTrayConut[4] = 9;                     // iAuto=1 -> iTrackPos=4
        AMR.ARTReset();
        CHECK(LastSet.bAMRLoaderLast == false, "ARTReset: bAMRLoaderLast cleared (golden :438)");
        CHECK(LastSet.iAMRTrayConut[4] == 1, "ARTReset: ART track collapsed to 1 cover tray (golden :443-445)");
        IniConfig.bA10_AutoReTest = false;
        TestIF_File.bSCKART_EnableART = false;
        BinSelect[iTestRunMode].bAutoRetest[1] = false;
    }

    // B11: NeedAMRTransport() (golden :449-465)
    printf("  [B11] NeedAMRTransport\n");
    {
        IniConfig.bA10_AutoReTest = false;
        ResetAMRLastSet();
        for (int i = 0; i < 9; i++) IniConfig.iA60NotifyQty[i] = 3;
        // NOTE: iAMRTrayConut[0]==0 (post-reset) already satisfies CheckLoaderCount's
        // "<=threshold" (golden :121), so "nothing pending" needs it ABOVE threshold
        // (and the Unloader counters, already 0, stay below their ">=threshold" gate).
        LastSet.iAMRTrayConut[0] = IniConfig.iA60NotifyQty[0] + 1;
        CHECK(AMR.NeedAMRTransport() == false, "NeedAMRTransport: nothing pending -> false (golden :465)");
        LastSet.bAMRTrayFeedWait = true;
        CHECK(AMR.NeedAMRTransport() == true, "NeedAMRTransport: bAMRTrayFeedWait -> true (golden :461)");
        LastSet.bAMRTrayFeedWait = false;
        LastSet.iAMRTrayConut[0] = 0;   // <= threshold -> CheckLoaderCount() true
        CHECK(AMR.NeedAMRTransport() == true, "NeedAMRTransport: CheckLoaderCount() true -> true (golden :457)");
    }

    // B12: SupplyCover() (golden :375-412) -- sim-faithful: TTrayMotor::SetTray in
    // the W4 offline sim only flips fHasTray (fHTary is always false offline, so
    // InitNewTray/data-fill never runs -- see Motor/mymotor.cpp SetTray). This
    // asserts the SIM-FAITHFUL outcome, not IC-data population.
    printf("  [B12] SupplyCover (sim-faithful: fHasTray flips, no IC-data fill)\n");
    {
        ResetAMRLastSet();
        IniConfig.bA10_AutoReTest = false;
        iTrayFeed = 0;
        MOT[MMAuto1_Car].fHasTray = false;
        MOT[MMAuto1].fHasTray = false;
        LastSet.iAMRTrayConut[3] = 0;   // iAuto==0 -> iTrackPos==3; ==0 && Car empty -> supply
        AMR.SupplyCover(0);
        CHECK(MOT[MMAuto1].fHasTray == true, "SupplyCover(0): empty-Car + 0 count -> MOT[MMAuto1].SetTray called (golden :383-386)");

        AMR.SupplyCover(9);   // invalid -> ShowMyMessage, no crash (golden :407-410)
        CHECK(true, "SupplyCover(9) invalid: no crash (golden :407-410 else)");
    }

    // =========================================================================
    //  PART C -- INTEGRATE-STEP PROOF: fAGV->IsSPIL_AMR/IsATK_AMR/Use_AMR now
    //  delegate to the REAL AGV_predicates.cpp functions (not the old hardcoded-
    //  false stand-ins).  This is the exact call shape acatchtray.cpp's ~18 call
    //  sites use (fAGV->IsATK_AMR() / fAGV->Use_AMR()), so seeding the same
    //  globals PART A already proved correct for the free functions and reading
    //  them back through fAGV directly demonstrates those call sites now take
    //  the REAL branch instead of the old always-false one.
    // =========================================================================
    printf("[C] FormsFacade fAGV-> delegation (acatchtray.cpp's exact call shape)\n");
    {
        USE_COVER_TRAYID = tCID_Keyence;
        IniConfig.bA65_BundleIDList = true;
        IniConfig.bSPILFunction     = true;
        CHECK(fAGV->IsSPIL_AMR() == true, "fAGV->IsSPIL_AMR(): Keyence+BundleID+SPIL -> true (was hardcoded false pre-integrate)");

        USE_COVER_TRAYID = tCID_NFC;
        CUSTOMER_CODE = CC_AMKOR_Korea;
        CHECK(fAGV->IsATK_AMR() == true, "fAGV->IsATK_AMR(): NFC+BundleID+AMKOR_Korea -> true (was hardcoded false pre-integrate)");
        CUSTOMER_CODE = 0;
        CHECK(fAGV->IsATK_AMR() == false, "fAGV->IsATK_AMR(): wrong customer -> false");

        USE_COVER_TRAYID = tCID_Keyence;
        CHECK(fAGV->Use_AMR() == true, "fAGV->Use_AMR(): Keyence+BundleID -> true (was hardcoded false pre-integrate)");
        USE_COVER_TRAYID = tCIDNotUse;
        CHECK(fAGV->Use_AMR() == false, "fAGV->Use_AMR(): tCIDNotUse -> false");
        USE_COVER_TRAYID = tCID_Keyence;
        IniConfig.bA65_BundleIDList = false;
    }

    // -------------------------------------------------------------------
    // Summary
    // -------------------------------------------------------------------
    printf("\n=== SUMMARY: %d passed, %d failed (total %d) ===\n",
           g_pass, g_fail, g_pass + g_fail);
    if (g_fail != 0)
    {
        printf("RESULT: FAIL\n");
        return 1;
    }
    printf("RESULT: ALL PASS\n");
    return 0;
}
