// =============================================================================
//  test_w906_autositemap_cleanout.cpp -- W906-AutoSiteMapCleanOut VERIFY:
//  InitCleanOutFunction's AutoSiteMap branch (newly un-gated) does REAL work.
//
//  Translation wave: W906-AutoSiteMapCleanOut (un-gate the AutoSiteMap branch
//                    of InitCleanOutFunction, golden csystem.cpp:15751-15785 --
//                    the last remaining piece of Wave16; siblings
//                    DoHotplateEdgeCylinderLoop + DoLoaderVibrateLoop landed
//                    20260722, commit 6f60737).
//  Author: AI(W906-AutoSiteMapCleanOut) 20260727
//  Suite name (add_test): W906_AutoSiteMapCleanOut
//
//  PURPOSE
//  -------
//  Before this wave, the AutoSiteMap branch of InitCleanOutFunction was
//  wrapped in `#if 0` -- calling InitCleanOutFunction() in AutoSiteMap mode
//  never touched iResetSiteMappingStep / bSiteMappingCHKOK / the per-plate
//  SiteMapData grid / bAutoSiteMapHotplateSave at all.  This TU proves the
//  now-active branch reproduces golden's REAL logic (golden csystem.cpp
//  :15751-15785), not just "compiles":
//    O1 GUARD false (iRunStartMode!=rsmAutoSiteMap, or Hot-temp/JCET guard
//       false) -> the whole branch is skipped: SiteMapData/iResetSiteMapping-
//       Step/bSiteMappingCHKOK/bAutoSiteMapHotplateSave all untouched. The
//       tail cursor resets (iHome/iReset/iCleanOut/iTrayFeed/bCleanoutStart)
//       still fire unconditionally (golden :15788-15792).
//    O2 GUARD true + bAutoSiteMapHasPickHP==true -> ONLY iResetSiteMappingStep
//       is set to 2 (golden :15757); the else-branch's SiteMapData zero-out /
//       bAutoSiteMapHotplateSave=false / bSiteMappingCHKOK are NOT touched.
//    O3 GUARD true + bAutoSiteMapHasPickHP==false -> the else-branch fires:
//       bSiteMappingCHKOK=true (golden :15761), and the nested
//       i<2 / j<HotPlateForm.XDivision / k<HotPlateForm.YDivision loop zeroes
//       MOT[MMPlate1+i].Tray.SiteMapData[j][k] for BOTH plates (golden
//       :15771-15780) -- proven by seeding non-zero cells both INSIDE and
//       OUTSIDE the XDivision/YDivision bounds on MMPlate1 AND MMPlate2, then
//       asserting only the in-bounds cells on both plates were zeroed (a
//       naive "wipe the whole array" translation would also zero the
//       out-of-bounds sentinels -- this test would catch that regression).
//       bAutoSiteMapHotplateSave is then set false (golden :15781) even when
//       seeded true beforehand.
//    O4 fMain->SetMainRunStartMode(...) / InitInArmTask() / ShowTestHeadComp()
//       are documented no-op stubs this wave (see FormsFacade.cpp /
//       aHotPlateSubstrate.cpp) -- calling the branch that reaches them must
//       not crash, and iAutoSiteMapRunStartMode (the value that selects which
//       constant is passed to the stub) is itself left unmutated by the call
//       (a stub has no side effect to observe here; this just documents the
//       gap is inert, not a hidden landmine).
//
//  EQUIVALENCE NOTE: no Borland binary exists, so "equivalence" == clean g++
//  compile/link + the branch's cursor/grid mutations match golden's
//  hand-derived guard/loop bounds exactly, with the fMain->SetMainRunStartMode
//  GAP stub proven inert (no crash, no unexpected mutation).
// =============================================================================
#include "csystem.h"               // InitCleanOutFunction
#include "canary_support.h"        // LastSet (LAST_GENERAL_SET)
#include "cmydef.h"                // rsmAutoSiteMap/rsmContinuStart/rsmContinuRetest,
                                    // Tempture_Hot, bAutoSiteMapHasPickHP,
                                    // iResetSiteMappingStep, bSiteMappingCHKOK,
                                    // iAutoSiteMapRunStartMode, bAutoSiteMapHotplateSave,
                                    // MMPlate1, iHome/iReset/iCleanOut/iTrayFeed/bCleanoutStart
#include "CosFunction.h"           // CosFunction.bUSEJCETSiteMapMode
#include "cprod.h"                 // HotPlateForm.XDivision/YDivision
#include "Motor/mymotor.h"         // MOT[]
#include <cstdio>
// AI(W906-P10) 20260921: asGeneralPath / OpenGeneralIniFile / CloseGeneralIniFile
#include "common.h"
#include <cstdlib>
#include "w906_test_tmpname.h"   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h)
#include "forms/fTemp_Set.h"   // AI(W906-N1G5) 20261008 (Ifor01): fTemp_Set (see main)

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// Fill both plates' SiteMapData with a non-zero sentinel everywhere so any
// zero-out (in-bounds or out-of-bounds) is observable.
static void SeedSiteMapData(int sentinel)
{
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < _MAX_COL_ITEM; ++j)
            for (int k = 0; k < _MAX_ROW_ITEM; ++k)
                MOT[MMPlate1+i].Tray.SiteMapData[j][k] = sentinel;
}

// AI(W906-P10) 20260921: 把 general ini 導到暫存檔 —— **絕不碰量產的
//   system\Gerneral.ini**（memory: ht9045-gerneral-ini-normalized-20260817）。
//   慣例照抄 tests/test_binsel_core.cpp:93-99 / :231-233。
static AnsiString AsmScratchIniPath(const char* leaf)
{
    const char* t = std::getenv("TEMP");
    if (!t || !*t) t = std::getenv("TMP");
    if (!t || !*t) t = ".";
    return AnsiString(t) + AnsiString("\\") + AnsiString(W906_TestTmpName(leaf).c_str());   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h)
}

int main()
{
    if (fTemp_Set == 0) fTemp_Set = new TfTemp_Set();   // AI(W906-N1G5) 20261008 (Ifor01): golden boot creates TfTemp_Set (CreateForm HT9045.cpp:186) before anything reaches ChangeSite, which calls fTemp_Set->InitialAddrToATC() since N1-G5
    // ★ AI(W906-P10) 20260921: 這幾行是 P10(2) 逼出來的，**與
    //   tests/test_SCK_ART_Remainder.cpp 同一個根因**（那是第一支，這是第二支）。
    //
    //   P10(2) 把 `TfMain::SetMainRunStartMode` 翻成真本體之後：
    //       InitCleanOutFunction -> SetMainRunStartMode -> SetRunStartMode
    //       -> TfBinSel::ReadFile -> SetWorkParameter -> ReadTechData
    //       -> CloseGeneralIniFile -> 對已釋放記憶體呼叫 -> SEGFAULT
    //
    //   `CloseGeneralIniFile()`（common.cpp:1559）在 delete 之後**刻意不把
    //   INIFileGeneral 設回 NULL**（註解寫 "faithful bug"），而 **golden 確實
    //   就是這樣**（golden common.cpp:1414-1421，逐行比對過）。
    //
    //   ⇒ 崩的不是翻譯，是測試行程沒有滿足 golden 在 main() 裡保證的前置。
    //   ⚠ **系統性問題**：任何走到 SetRunStartMode 的測試都會中。
    //     真正的解法是把那個 dangling pointer 修掉，但那是**偏離 golden**，
    //     依 CLAUDE.md「改行為須使用者決定」要先問過 —— 已記在 INBOX。
    AnsiString savedGeneralPath = asGeneralPath;
    asGeneralPath = AsmScratchIniPath("test_asm_cleanout_general.ini");
    OpenGeneralIniFile();
    printf("==== W906-AutoSiteMapCleanOut verify (InitCleanOutFunction AutoSiteMap branch) ====\n");

    // --- save every global the branch / oracle touches ----------------------
    const int  saveRunStartMode      = LastSet.iRunStartMode;
    const int  saveTemperature       = LastSet.iTemperature;
    const bool saveJCETMode          = CosFunction.bUSEJCETSiteMapMode;
    const bool savePickHP            = bAutoSiteMapHasPickHP;
    const int  saveResetStep         = iResetSiteMappingStep;
    const bool saveSiteMapCHKOK      = bSiteMappingCHKOK;
    const int  saveAutoRunStartMode  = iAutoSiteMapRunStartMode;
    const bool saveHotplateSave      = bAutoSiteMapHotplateSave;
    const int  saveXDivision         = HotPlateForm.XDivision;
    const int  saveYDivision         = HotPlateForm.YDivision;
    const int  saveFileXDivision     = HotPlateForm_File.XDivision;
    const int  saveFileYDivision     = HotPlateForm_File.YDivision;
    const int  saveHome              = iHome;
    const int  saveReset             = iReset;
    const int  saveCleanOut          = iCleanOut;
    const int  saveTrayFeed          = iTrayFeed;
    const bool saveCleanoutStart     = bCleanoutStart;

    // Small, deterministic in-bounds window (well inside _MAX_COL_ITEM=30 /
    // _MAX_ROW_ITEM=70), so the seeded out-of-bounds sentinels below stay put.
    HotPlateForm.XDivision = 3;
    HotPlateForm.YDivision = 2;
    // AI(W906-TRAY-READ) 20260923: 同時設 _File 那份。T5 解開 N3-G8 之後，O3 的
    //   InitCleanOutFunction -> SetMainRunStartMode -> SetRunStartMode -> SetWorkParameter
    //   -> DoHotPlateConvert 會 memcpy(HotPlateForm, HotPlateForm_File)（cUnitConvert.cpp:444，
    //   golden 同），而且發生在 csystem.cpp 那個有界清除迴圈**之前** —— 不設的話 XDivision/YDivision
    //   被蓋成 0，O3d 跑 0 圈而恆真、O3e 退化成只看 [0][0]，測試會綠但什麼都沒驗（20260923 審查 M2）。
    HotPlateForm_File.XDivision = 3;
    HotPlateForm_File.YDivision = 2;

    // =========================================================================
    //  O1 -- GUARD false: whole branch skipped, tail resets still fire
    // =========================================================================
    printf("[O1] guard false (iRunStartMode!=rsmAutoSiteMap) -> branch skipped (golden :15751-15753)\n");
    SeedSiteMapData(7);
    LastSet.iRunStartMode = rsmContinuStart;         // != rsmAutoSiteMap
    LastSet.iTemperature  = Tempture_Hot;
    CosFunction.bUSEJCETSiteMapMode = true;
    bAutoSiteMapHasPickHP = false;
    iResetSiteMappingStep = 111;                     // sentinel: must survive
    bSiteMappingCHKOK     = false;                   // sentinel: must survive
    bAutoSiteMapHotplateSave = true;                 // sentinel: must survive
    iHome = 9; iReset = 9; iCleanOut = 9; iTrayFeed = 9; bCleanoutStart = false;

    // AI(W906-P10) 20260921: **每次呼叫前都要重開** —— 一次開在 main 開頭不夠。
    //   這條鏈每跑一次就 CloseGeneralIniFile() 一次，而它 delete 後不把
    //   INIFileGeneral 設回 NULL（golden 亦然），所以第 2 次就踩懸空指標。
    //   重開會配置新物件並覆寫那個指標，是這裡最小且安全的作法。
    OpenGeneralIniFile();
    InitCleanOutFunction();

    CHECK(iResetSiteMappingStep == 111,
          "O1a guard false -> iResetSiteMappingStep untouched");
    CHECK(bSiteMappingCHKOK == false,
          "O1b guard false -> bSiteMappingCHKOK untouched");
    CHECK(bAutoSiteMapHotplateSave == true,
          "O1c guard false -> bAutoSiteMapHotplateSave untouched");
    CHECK(MOT[MMPlate1].Tray.SiteMapData[0][0] == 7 &&
          MOT[MMPlate1+1].Tray.SiteMapData[0][0] == 7,
          "O1d guard false -> SiteMapData untouched on both plates");
    CHECK(iHome == 0 && iReset == 0 && iCleanOut == 1 && iTrayFeed == 0 && bCleanoutStart == true,
          "O1e tail cursor resets still fire unconditionally (golden :15788-15792)");

    // =========================================================================
    //  O2 -- GUARD true + bAutoSiteMapHasPickHP==true -> ONLY iResetSiteMappingStep=2
    // =========================================================================
    printf("[O2] guard true + bAutoSiteMapHasPickHP==true -> only iResetSiteMappingStep=2 (golden :15755-15757)\n");
    SeedSiteMapData(7);
    LastSet.iRunStartMode = rsmAutoSiteMap;
    LastSet.iTemperature  = Tempture_Hot;
    CosFunction.bUSEJCETSiteMapMode = true;
    bAutoSiteMapHasPickHP = true;
    iResetSiteMappingStep = 0;
    bSiteMappingCHKOK     = false;                   // sentinel: must NOT become true on this path
    bAutoSiteMapHotplateSave = true;                 // sentinel: must NOT become false on this path

    // AI(W906-P10) 20260921: **每次呼叫前都要重開** —— 一次開在 main 開頭不夠。
    //   這條鏈每跑一次就 CloseGeneralIniFile() 一次，而它 delete 後不把
    //   INIFileGeneral 設回 NULL（golden 亦然），所以第 2 次就踩懸空指標。
    //   重開會配置新物件並覆寫那個指標，是這裡最小且安全的作法。
    OpenGeneralIniFile();
    InitCleanOutFunction();

    CHECK(iResetSiteMappingStep == 2,
          "O2a bAutoSiteMapHasPickHP==true -> iResetSiteMappingStep==2 (golden :15757)");
    CHECK(bSiteMappingCHKOK == false,
          "O2b bAutoSiteMapHasPickHP==true -> bSiteMappingCHKOK NOT touched (else-branch only, golden :15761)");
    CHECK(bAutoSiteMapHotplateSave == true,
          "O2c bAutoSiteMapHasPickHP==true -> bAutoSiteMapHotplateSave NOT touched (else-branch only, golden :15781)");
    CHECK(MOT[MMPlate1].Tray.SiteMapData[0][0] == 7 &&
          MOT[MMPlate1+1].Tray.SiteMapData[0][0] == 7,
          "O2d bAutoSiteMapHasPickHP==true -> SiteMapData NOT touched (else-branch only, golden :15771-15780)");

    // =========================================================================
    //  O3 -- GUARD true + bAutoSiteMapHasPickHP==false -> else-branch REAL work
    // =========================================================================
    printf("[O3] guard true + bAutoSiteMapHasPickHP==false -> else-branch fires (golden :15759-15784)\n");
    SeedSiteMapData(7);   // both plates, ALL cells (in- and out-of-bounds) = 7
    LastSet.iRunStartMode = rsmAutoSiteMap;
    LastSet.iTemperature  = Tempture_Hot;
    CosFunction.bUSEJCETSiteMapMode = true;
    bAutoSiteMapHasPickHP = false;
    bSiteMappingCHKOK        = false;
    bAutoSiteMapHotplateSave = true;
    iAutoSiteMapRunStartMode = 0;   // selects fMain->SetMainRunStartMode(rsmContinuStart) -- stub, inert

    // AI(W906-P10) 20260921: **每次呼叫前都要重開** —— 一次開在 main 開頭不夠。
    //   這條鏈每跑一次就 CloseGeneralIniFile() 一次，而它 delete 後不把
    //   INIFileGeneral 設回 NULL（golden 亦然），所以第 2 次就踩懸空指標。
    //   重開會配置新物件並覆寫那個指標，是這裡最小且安全的作法。
    OpenGeneralIniFile();
    InitCleanOutFunction();

    CHECK(bSiteMappingCHKOK == true,
          "O3a else-branch -> bSiteMappingCHKOK=true (golden :15761)");
    CHECK(bAutoSiteMapHotplateSave == false,
          "O3b else-branch -> bAutoSiteMapHotplateSave=false (golden :15781)");
    CHECK(iAutoSiteMapRunStartMode == 0,
          "O3c fMain->SetMainRunStartMode is a documented no-op stub -- selector left unmutated (no crash either)");

    // AI(W906-TRAY-READ) 20260923: 先斷言迴圈邊界沒有被蓋成 0 —— 否則下面 O3d 跑 0 圈而恆真
    //   （見上面 HotPlateForm_File 那段）。這一條失敗 = O3d/O3e 的結果不可信。
    CHECK(HotPlateForm.XDivision == 3 && HotPlateForm.YDivision == 2,
          "O3-pre loop bounds survived the SetWorkParameter chain (XDivision=3, YDivision=2) -- else O3d/O3e would be vacuous");

    // In-bounds cells (j<XDivision=3, k<YDivision=2) on BOTH plates must be
    // zeroed (golden :15771-15780 loops i<2).
    bool inBoundsZeroed = true;
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < HotPlateForm.XDivision; ++j)
            for (int k = 0; k < HotPlateForm.YDivision; ++k)
                if (MOT[MMPlate1+i].Tray.SiteMapData[j][k] != 0)
                    inBoundsZeroed = false;
    CHECK(inBoundsZeroed,
          "O3d in-bounds SiteMapData[j<XDivision][k<YDivision] zeroed on BOTH plates (golden :15771-15780)");

    // Out-of-bounds sentinels (j>=XDivision or k>=YDivision) must survive --
    // a naive "wipe the whole array" mistranslation would fail this.
    CHECK(MOT[MMPlate1].Tray.SiteMapData[HotPlateForm.XDivision][0] == 7 &&
          MOT[MMPlate1].Tray.SiteMapData[0][HotPlateForm.YDivision] == 7 &&
          MOT[MMPlate1+1].Tray.SiteMapData[HotPlateForm.XDivision][0] == 7 &&
          MOT[MMPlate1+1].Tray.SiteMapData[0][HotPlateForm.YDivision] == 7,
          "O3e out-of-bounds SiteMapData sentinels survive on BOTH plates (exact XDivision/YDivision bounds preserved, not a full-array wipe)");

    // =========================================================================
    //  O3-alt -- iAutoSiteMapRunStartMode!=0 selects the OTHER stub constant
    //  (golden :15766-15769); still fully inert (documented GAP), no crash.
    // =========================================================================
    printf("[O3-alt] iAutoSiteMapRunStartMode!=0 -> other SetMainRunStartMode stub arg selected, still inert (golden :15766-15769)\n");
    SeedSiteMapData(9);
    // AI(W906-P10) 20260921: **補上 LastSet.iRunStartMode** —— 這一格原本漏了，
    //   而樁把它掩蓋了。O3-alt 自己的說明是「只換 iAutoSiteMapRunStartMode 這一個
    //   變數」，但它沒有像 O3 那樣重設 LastSet.iRunStartMode，於是沿用上一段被
    //   SetRunStartMode 改掉的值。
    //   樁時代沒差（SetMainRunStartMode 是 no-op）；P10(2) 之後就有差：
    //   `RunStartMode.cpp:705 if(LastSet.iRunStartMode!=rsmAutoSiteMap)` 會成立，
    //   :724 把 bSiteMappingCHKOK 設回 false，斷言就掛。
    //   ⇒ 補這一行是**還原這一格的本意**（單變數對照），不是把斷言改弱。
    LastSet.iRunStartMode    = rsmAutoSiteMap;
    bSiteMappingCHKOK        = false;
    bAutoSiteMapHotplateSave = true;
    iAutoSiteMapRunStartMode = 1;   // selects fMain->SetMainRunStartMode(rsmContinuRetest)

    // AI(W906-P10) 20260921: **每次呼叫前都要重開** —— 一次開在 main 開頭不夠。
    //   這條鏈每跑一次就 CloseGeneralIniFile() 一次，而它 delete 後不把
    //   INIFileGeneral 設回 NULL（golden 亦然），所以第 2 次就踩懸空指標。
    //   重開會配置新物件並覆寫那個指標，是這裡最小且安全的作法。
    OpenGeneralIniFile();
    InitCleanOutFunction();

    CHECK(bSiteMappingCHKOK == true && bAutoSiteMapHotplateSave == false,
          "O3-alt else-branch real mutations fire the same regardless of which stub arg was selected");

    // --- restore every seeded global -----------------------------------------
    LastSet.iRunStartMode    = saveRunStartMode;
    LastSet.iTemperature     = saveTemperature;
    CosFunction.bUSEJCETSiteMapMode = saveJCETMode;
    bAutoSiteMapHasPickHP    = savePickHP;
    iResetSiteMappingStep    = saveResetStep;
    bSiteMappingCHKOK        = saveSiteMapCHKOK;
    iAutoSiteMapRunStartMode = saveAutoRunStartMode;
    bAutoSiteMapHotplateSave = saveHotplateSave;
    HotPlateForm.XDivision   = saveXDivision;
    HotPlateForm.YDivision   = saveYDivision;
    HotPlateForm_File.XDivision = saveFileXDivision;
    HotPlateForm_File.YDivision = saveFileYDivision;
    iHome = saveHome; iReset = saveReset; iCleanOut = saveCleanOut;
    iTrayFeed = saveTrayFeed; bCleanoutStart = saveCleanoutStart;

    printf("==== W906-AutoSiteMapCleanOut verify: %d passed, %d failed ====\n",
           g_pass, g_fail);
    // AI(W906-P10) 20260921: 收尾還原（見 main() 開頭的說明）
    CloseGeneralIniFile();
    asGeneralPath = savedGeneralPath;
    std::remove(AsmScratchIniPath("test_asm_cleanout_general.ini").c_str());   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h): remove this run's scratch ini

    return (g_fail == 0) ? 0 : 1;
}
