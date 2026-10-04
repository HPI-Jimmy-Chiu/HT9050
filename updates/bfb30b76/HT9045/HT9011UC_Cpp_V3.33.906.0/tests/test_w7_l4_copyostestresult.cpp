// ---------------------------------------------------------------------------
//  test_w7_l4_copyostestresult.cpp -- Interface/TesterTCP.cpp
//  TesterTCP_CopyOSTestResult (golden Interface/TesterTCP.cpp:660-697, 38 lines)
// ---------------------------------------------------------------------------
//  AI(W906-W7-L4-fix) 20260802: REWRITTEN. The previous version of this suite
//  observed one thing only -- "did the directory D:\HT9045_Log\OSTestResult
//  appear on this disk" -- and an independent review demonstrated by direct
//  experiment that this made it largely vacuous: two real mutations of the
//  translated body survived it with 3 PASS / 0 FAIL, unchanged.
//    * dropping the golden `-1` from `iTesterCh = GetSiteNo()-1` (golden :671)
//    * deleting the `if(CheckFileExist(...))` guard at golden :681 so CopyFile
//      runs unconditionally
//  Root cause: this machine has no Z: drive, so CheckFileExist("Z:\Device<N>
//  .TXT") was false for every <N>, CopyFile never ran, and every assertion
//  about the copy held vacuously. The same review also found two FALSE
//  SELF-CLAIMS in the old header (that every counted CHECK was a conjunction a
//  neutered body must fail, and that an unconditional CopyFile "would go false
//  on its own"); mutation C disproved the second outright. Both claims are
//  gone, not softened.
//
//  WHAT THIS SUITE NOW DOES. Interface/TesterTCP.{h,cpp} now route the three
//  environment-touching calls through recording seams with test-installable
//  hooks (see the seam banner in TesterTCP.h). This suite drives the function
//  entirely through those seams, so it can pin the exact TARGET FILENAME
//  golden builds --
//      "D:\HT9045_Log\OSTestResult\Device%06d_%02d.TXT", iContactIndex, iTesterCh
//  -- which is the ONLY externally visible place iContactIndex and iTesterCh
//  (and therefore the golden `-1`) appear.
//
//  WHAT EACH PART IS FOR, honestly:
//    PART 0  INFO ONLY, never counted -- GetSiteNo/GetOrderOfContact sanity.
//    PART 1  gate #1 (iTestType) -- closed: nothing recorded. open: force-
//            directories recorded with the exact golden path and tag.
//    PART 2  gate #2 (LastSet.iTester, SOFT_SIMULTE off) -- same shape.
//    PART 3  both gates open, existence hook says FALSE: exactly 2 probes, with
//            the exact golden probe paths, and ZERO copies.
//    PART 4  both gates open, existence hook says TRUE: exactly 2 copies, each
//            source and target pinned to the exact literal string, including
//            the golden double-write quirk (both copies target the SAME name).
//    PART 5  a second fixture (site 1 -> channel 00) so the `-1` is pinned at
//            the %02d zero-padding boundary as well.
//    PART 6  existence hook TRUE only for the FailPin probe: proves the two
//            guards are independent, not one shared gate.
//    PART 7  production pass-through proof: with NO hook installed the seams
//            really call the real MyForceDirectories / CheckFileExist /
//            CopyFile. Demonstrated on scratch temp paths through the three
//            W7L4TesterTCP_Prove*Seam entry points, then the same call is
//            shown to divert once a hook IS installed.
//
//  WHICH CHECKS SURVIVE A FULLY NEUTERED (immediate-return) BODY, stated
//  exactly rather than dressed up: PART1a, PART2a and PART3d are all
//  "...therefore nothing was recorded" assertions, which an empty body
//  satisfies vacuously; and all five PART 7 checks exercise the seams
//  directly, not the body, so they are insensitive to it by construction.
//  Every OTHER counted CHECK in this file (PART1b, PART2b, PART3a-c,
//  PART4a-f, PART5, PART6 -- 12 of the 21) goes red on a neutered body.
//  Measured, not assumed: 21 PASS / 0 FAIL baseline, 9 PASS / 12 FAIL with an
//  immediate `return;` at the top of TesterTCP_CopyOSTestResult.
//
//  THIS SUITE TOUCHES NO PRODUCTION DIRECTORY. PARTS 1-6 install a
//  force-directories hook and a copy hook, so nothing under D:\HT9045_Log is
//  ever created, probed or written. PART 7 writes only inside a scratch
//  directory it creates under %TEMP% (or the current directory as fallback)
//  and removes again.
//
//  LIMITATIONS (real gaps, stated plainly, not worked around):
//    L1  The SOFT_SIMULTE branch (golden :677/:688, sources under D:\RMS\) is
//        NOT exercised. This build has SOFT_SIMULTE undefined -- the #error
//        guard below enforces that -- so the pinned source strings are the
//        Z:\ ones. A build that defines SOFT_SIMULTE needs the expected source
//        strings re-derived; the guard makes that impossible to miss.
//    L2  No real Z: drive and no real D:\HT9045_Log\OSTestResult are involved
//        anywhere, by design. This suite therefore proves WHAT ARGUMENTS golden
//        passes to CopyFile, not that a real Windows CopyFile of a real Z:
//        file succeeds. The real-CopyFile leg is covered separately, on scratch
//        paths, by PART 7.
//    L3  TesterTCP_CopyOSTestResult still has no production caller in this
//        tree (see the reachability banner at its body). This suite calls it
//        directly; it cannot and does not prove any production path reaches it.
//    L4  Only TesterTCP_CopyOSTestResult is in scope. The sibling
//        PlaceOSTestResultToTray remains untranslated and untested -- see
//        Interface/TesterTCP.h's SCOPE banner.
//
//  No external test framework: a tiny check harness prints PASS/FAIL per case
//  and a final summary, and returns non-zero on ANY failure.
// ---------------------------------------------------------------------------

#include "Interface/TesterTCP.h"

#include "MachineDefine.h"
#include "MachineType.h"
#include "cprod.h"
#include "cmydef.h"
#include "canary_support.h"
#include "aHotPlateSubstrate.h"
#include "Public/MyProductionRecord.h"

#include <cstdio>
#include <cstdlib>
#include "w906_test_tmpname.h"   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h)

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("PASS  %s\n", msg); ++g_pass; }                 \
        else      { std::printf("FAIL  %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

//AI(W906-ST-S3-B2b) 20260918: the `#error` that stood here is replaced by a
//  RUNTIME SKIP. Same reasoning as tests/test_w7_l2_ckernel.cpp's guard; read
//  that one first, it carries the long version.
//
//  THE TRIPWIRE WAS RIGHT. This suite pins source-path strings and holds gate #2
//  open via LastSet.iTester==ON_LINE, both derived with SOFT_SIMULTE OFF
//  (LIMITATION L1). With it on, golden takes different arms and those pinned
//  values are no longer the values under test -- the suite would still pass, and
//  that pass would mean nothing.
//
//  The user turned SOFT_SIMULTE on 20260918 and asked for blockers to be MARKED
//  rather than solved ("後續遇到先 mark 不處理"), with the analysis written down.
//  So: the whole suite reports SKIPPED instead of asserting on stale pins.
//
//  NOT COVERED WHILE THIS IS ON: CopyOSTestResult's two gates and its pinned
//  source paths.
//  TO RESTORE: re-derive the pinned strings and gate #2 against golden's
//  SOFT_SIMULTE arm, then delete the #define below.
#ifdef SOFT_SIMULTE
#define W906_SKIP_SUITE_SOFT_SIMULTE 1
#endif

// ---------------------------------------------------------------------------
//  Hooks installed into the Interface/TesterTCP.cpp seams.
// ---------------------------------------------------------------------------

// Force-directories: never create anything, just report golden's own success
// code (Gated_MyForceDirectories returns 1 on success). CopyOSTestResult
// discards the return value anyway (golden :674 does too).
static int HookForceDirs_NoOp(AnsiString /*Directory*/, AnsiString /*Function*/)
{
    return 1;
}

// Copy: never touch the filesystem. The seam has already recorded both
// arguments by the time this runs, which is what the assertions read.
static bool HookCopy_NoOp(AnsiString /*sSource*/, AnsiString /*sTarget*/, bool /*bFailIfExists*/)
{
    return true;
}

static bool HookExist_AlwaysFalse(AnsiString /*cFName*/)
{
    return false;
}

static bool HookExist_AlwaysTrue(AnsiString /*cFName*/)
{
    return true;
}

// True only for golden's SECOND source (golden :690, "Z:\FailPin_Device<N>.TXT").
// AnsiString::Pos is 1-based; 0 means "not found".
static bool HookExist_OnlyFailPin(AnsiString cFName)
{
    return (cFName.Pos("FailPin") > 0);
}

static void InstallNonFilesystemHooks(W7L4TesterTCP_CheckFileExistHookFn existHook)
{
    W7L4TesterTCP_ForceDirectoriesHook = HookForceDirs_NoOp;
    W7L4TesterTCP_CopyFileHook         = HookCopy_NoOp;
    W7L4TesterTCP_CheckFileExistHook   = existHook;
}

static void UninstallAllHooks()
{
    W7L4TesterTCP_ForceDirectoriesHook = NULL;
    W7L4TesterTCP_CopyFileHook         = NULL;
    W7L4TesterTCP_CheckFileExistHook   = NULL;
}

// Seeds the PordRec cell CopyOSTestResult reads (golden :671-672).
static void SeedPordRec(int r, int c, int iSiteNo, int iOrderOfContact)
{
    TestSocket.PordRec[r][c].asBuffer->Strings[eSiteNO]    = AnsiString(iSiteNo);
    TestSocket.PordRec[r][c].asBuffer->Strings[eOrderTest] = AnsiString(iOrderOfContact);
}

int main()
{
    std::printf("=== test_w7_l4_copyostestresult ===\n");

#ifdef W906_SKIP_SUITE_SOFT_SIMULTE
    // AI(W906-ST-S3-B2b) 20260918: see the guard note near the top of this file.
    //   Exits 0 on purpose: this is a SKIP, not a pass and not a failure. The
    //   line below is the only thing that makes the skip visible, so do not
    //   quieten it -- a silent skip is indistinguishable from a suite that ran.
    std::printf("SKIP  whole suite: SOFT_SIMULTE is defined. The pinned source-path\n"
                "      strings and gate #2 (LastSet.iTester) were derived with it OFF\n"
                "      (LIMITATION L1); asserting on them now would pass without\n"
                "      testing anything. Re-derive them before removing this skip.\n");
    return 0;
#endif

    const int savedTestType = TestIF_File.iTestType;
    const int savedTester   = LastSet.iTester;

    // -----------------------------------------------------------------
    // PART 0 -- INFO ONLY, never counted. Direct sanity check of the two
    // leaves CopyOSTestResult's iTesterCh/iContactIndex arithmetic depends on
    // (golden Interface/TesterTCP.cpp:671-672). They are a DIFFERENT,
    // already-translated unit (also covered by tests/test_MyProductionRecord
    // .cpp), so neutering CopyOSTestResult cannot and should not turn this
    // red -- it is printed as INFO and kept out of the tally.
    //
    // AI(W906-W7-L4-fix) 20260802: citation corrected. This used to be labelled
    // "golden .cpp:666-673", which was a PORTED line range attributed to
    // golden. Verified against both trees this pass:
    //   GetSiteNo         PORTED Public/MyProductionRecord.cpp:666-669
    //                     GOLDEN Public/MyProductionRecord.cpp:519-522
    //   GetOrderOfContact PORTED Public/MyProductionRecord.cpp:671-674
    //                     GOLDEN Public/MyProductionRecord.cpp:524-527
    // GOLDEN Public/MyProductionRecord.cpp:666-673 is the unrelated
    // TMyProductionRecord::SaveRecord, not either of these.
    // -----------------------------------------------------------------
    std::printf("\n-- PART 0 (info only, not counted): GetSiteNo/GetOrderOfContact sanity --\n");
    {
        TMyProductionRecord rec;
        rec.asBuffer->Strings[eSiteNO]    = AnsiString(7);
        rec.asBuffer->Strings[eOrderTest] = AnsiString(123);
        bool ok = (rec.GetSiteNo() == 7) && (rec.GetOrderOfContact() == 123);
        std::printf("%s  PART0: GetSiteNo()==7 && GetOrderOfContact()==123 for seeded eSiteNO/eOrderTest"
                    " (ported Public/MyProductionRecord.cpp:666-674, golden Public/MyProductionRecord.cpp:519-527)\n",
                    ok ? "INFO-OK" : "INFO-FAIL");
    }

    // The fixture PARTS 1-4 and 6 use: site 7 -> golden iTesterCh 7-1 == 6,
    // contact 123. Chosen so the golden `-1` changes BOTH the probe path
    // ("Z:\Device6.TXT" vs "Z:\Device7.TXT") AND the target filename
    // ("..._06.TXT" vs "..._07.TXT").
    const int r = 0, c = 0;
    SeedPordRec(r, c, 7, 123);

    // -----------------------------------------------------------------
    // PART 1 -- gate #1, TestIF_File.iTestType==TCP_IP_MODE (golden :662-663).
    // -----------------------------------------------------------------
    std::printf("\n-- PART 1: gate #1 (iTestType==TCP_IP_MODE) --\n");
    {
        InstallNonFilesystemHooks(HookExist_AlwaysFalse);

        W7L4TesterTCP_ResetSeamRecording();
        TestIF_File.iTestType = 0;          // anything other than TCP_IP_MODE
        LastSet.iTester       = ON_LINE;    // gate #2 held open
        TesterTCP_CopyOSTestResult(r, c);
        CHECK(W7L4TesterTCP_ForceDirectoriesCount == 0 &&
              W7L4TesterTCP_CheckFileExistCount   == 0 &&
              W7L4TesterTCP_CopyFileCount         == 0,
              "PART1a: iTestType!=TCP_IP_MODE -> early return, nothing at all is attempted (golden :662-663)");

        W7L4TesterTCP_ResetSeamRecording();
        TestIF_File.iTestType = TCP_IP_MODE;
        LastSet.iTester       = ON_LINE;
        TesterTCP_CopyOSTestResult(r, c);
        CHECK(W7L4TesterTCP_ForceDirectoriesCount == 1 &&
              W7L4TesterTCP_LastForceDirectoriesPath     == "D:\\HT9045_Log\\OSTestResult" &&
              W7L4TesterTCP_LastForceDirectoriesFunction == "CopyOSTestResult",
              "PART1b: iTestType==TCP_IP_MODE -> MyForceDirectories(\"D:\\HT9045_Log\\OSTestResult\",\"CopyOSTestResult\") exactly once (golden :674)");

        UninstallAllHooks();
    }

    // -----------------------------------------------------------------
    // PART 2 -- gate #2, LastSet.iTester==ON_LINE (golden :665-668, live in
    // this build because SOFT_SIMULTE is undefined -- see the #error guard).
    // -----------------------------------------------------------------
    std::printf("\n-- PART 2: gate #2 (LastSet.iTester==ON_LINE, SOFT_SIMULTE off) --\n");
    {
        InstallNonFilesystemHooks(HookExist_AlwaysFalse);

        W7L4TesterTCP_ResetSeamRecording();
        TestIF_File.iTestType = TCP_IP_MODE;   // gate #1 held open
        LastSet.iTester       = ON_LINE + 1;   // anything other than ON_LINE
        TesterTCP_CopyOSTestResult(r, c);
        CHECK(W7L4TesterTCP_ForceDirectoriesCount == 0 &&
              W7L4TesterTCP_CheckFileExistCount   == 0 &&
              W7L4TesterTCP_CopyFileCount         == 0,
              "PART2a: LastSet.iTester!=ON_LINE -> early return, nothing at all is attempted (golden :665-668)");

        W7L4TesterTCP_ResetSeamRecording();
        TestIF_File.iTestType = TCP_IP_MODE;
        LastSet.iTester       = ON_LINE;
        TesterTCP_CopyOSTestResult(r, c);
        CHECK(W7L4TesterTCP_ForceDirectoriesCount == 1 &&
              W7L4TesterTCP_LastForceDirectoriesPath == "D:\\HT9045_Log\\OSTestResult",
              "PART2b: LastSet.iTester==ON_LINE -> the force-directories side effect really happens (golden :674)");

        UninstallAllHooks();
    }

    // -----------------------------------------------------------------
    // PART 3 -- both gates open, existence hook answers FALSE for everything.
    // Pins the two probe paths exactly (each carries iTesterCh, so the golden
    // `-1` is pinned here too) and pins that NO copy happens at all.
    // -----------------------------------------------------------------
    std::printf("\n-- PART 3: both gates open, no source exists -> 2 probes, 0 copies --\n");
    {
        InstallNonFilesystemHooks(HookExist_AlwaysFalse);
        W7L4TesterTCP_ResetSeamRecording();

        TestIF_File.iTestType = TCP_IP_MODE;
        LastSet.iTester       = ON_LINE;
        TesterTCP_CopyOSTestResult(r, c);

        CHECK(W7L4TesterTCP_CheckFileExistCount == 2,
              "PART3a: exactly 2 existence probes (golden :681 and :692)");
        CHECK(W7L4TesterTCP_CheckFileExistPath[0] == "Z:\\Device6.TXT",
              "PART3b: probe #1 is exactly \"Z:\\Device6.TXT\" for GetSiteNo()==7 -- pins iTesterCh=GetSiteNo()-1 (golden :671/:679)");
        CHECK(W7L4TesterTCP_CheckFileExistPath[1] == "Z:\\FailPin_Device6.TXT",
              "PART3c: probe #2 is exactly \"Z:\\FailPin_Device6.TXT\" (golden :690)");
        CHECK(W7L4TesterTCP_CopyFileCount == 0,
              "PART3d: no source exists -> CopyFile is NEVER called -- pins that golden's if(CheckFileExist(...)) guards are real (golden :681/:692)");

        UninstallAllHooks();
    }

    // -----------------------------------------------------------------
    // PART 4 -- both gates open, existence hook answers TRUE for everything:
    // golden's copy branch, driven with no Z: drive and no filesystem writes.
    // This is where the target FILENAME gets pinned character for character.
    // -----------------------------------------------------------------
    std::printf("\n-- PART 4: both gates open, both sources exist -> the exact CopyFile arguments --\n");
    {
        InstallNonFilesystemHooks(HookExist_AlwaysTrue);
        W7L4TesterTCP_ResetSeamRecording();

        TestIF_File.iTestType = TCP_IP_MODE;
        LastSet.iTester       = ON_LINE;
        TesterTCP_CopyOSTestResult(r, c);

        // golden :683/:694 with iContactIndex==123 (%06d) and iTesterCh==6 (%02d).
        const AnsiString kTarget = "D:\\HT9045_Log\\OSTestResult\\Device000123_06.TXT";

        CHECK(W7L4TesterTCP_CopyFileCount == 2,
              "PART4a: both sources exist -> exactly 2 CopyFile calls (golden :684 and :695)");
        CHECK(W7L4TesterTCP_CopyFileSource[0] == "Z:\\Device6.TXT",
              "PART4b: copy #1 source is exactly \"Z:\\Device6.TXT\" (golden :679)");
        CHECK(W7L4TesterTCP_CopyFileTarget[0] == kTarget,
              "PART4c: copy #1 target is exactly \"D:\\HT9045_Log\\OSTestResult\\Device000123_06.TXT\" -- THE assertion that pins iContactIndex(%06d) and iTesterCh=GetSiteNo()-1(%02d) (golden :683)");
        CHECK(W7L4TesterTCP_CopyFileSource[1] == "Z:\\FailPin_Device6.TXT",
              "PART4d: copy #2 source is exactly \"Z:\\FailPin_Device6.TXT\" (golden :690)");
        CHECK(W7L4TesterTCP_CopyFileTarget[1] == kTarget,
              "PART4e: GOLDEN QUIRK PRESERVED -- copy #2 writes the SAME target name as copy #1, so a present FailPin file silently overwrites the Device file (golden :694 == :683, never \"fixed\")");
        CHECK(W7L4TesterTCP_CopyFileCount == 2 &&
              W7L4TesterTCP_CopyFileFailIfExists[0] == false &&
              W7L4TesterTCP_CopyFileFailIfExists[1] == false,
              "PART4f: both copies pass bFailIfExists==false, which is what lets the golden overwrite above happen (golden :684/:695)");

        UninstallAllHooks();
    }

    // -----------------------------------------------------------------
    // PART 5 -- second fixture at the %02d zero-padding boundary: site 1 is
    // the realistic first site, so golden's `-1` makes channel 0 and the name
    // must end "_00.TXT". A body missing the `-1` produces "_01.TXT".
    // -----------------------------------------------------------------
    std::printf("\n-- PART 5: second fixture, site 1 -> channel 00 --\n");
    {
        SeedPordRec(r, c, 1, 1);
        InstallNonFilesystemHooks(HookExist_AlwaysTrue);
        W7L4TesterTCP_ResetSeamRecording();

        TestIF_File.iTestType = TCP_IP_MODE;
        LastSet.iTester       = ON_LINE;
        TesterTCP_CopyOSTestResult(r, c);

        CHECK(W7L4TesterTCP_CopyFileCount == 2 &&
              W7L4TesterTCP_CopyFileSource[0] == "Z:\\Device0.TXT" &&
              W7L4TesterTCP_CopyFileTarget[0] == "D:\\HT9045_Log\\OSTestResult\\Device000001_00.TXT",
              "PART5: GetSiteNo()==1 -> source \"Z:\\Device0.TXT\" and target \"...Device000001_00.TXT\" -- the golden -1 at the zero boundary (golden :671/:679/:683)");

        UninstallAllHooks();
        SeedPordRec(r, c, 7, 123);   // restore the main fixture for PART 6
    }

    // -----------------------------------------------------------------
    // PART 6 -- existence hook TRUE only for the FailPin source: the two
    // golden guards are independent, not one shared decision.
    // -----------------------------------------------------------------
    std::printf("\n-- PART 6: only the FailPin source exists -> exactly 1 copy, the second one --\n");
    {
        InstallNonFilesystemHooks(HookExist_OnlyFailPin);
        W7L4TesterTCP_ResetSeamRecording();

        TestIF_File.iTestType = TCP_IP_MODE;
        LastSet.iTester       = ON_LINE;
        TesterTCP_CopyOSTestResult(r, c);

        CHECK(W7L4TesterTCP_CheckFileExistCount == 2 &&
              W7L4TesterTCP_CopyFileCount       == 1 &&
              W7L4TesterTCP_CopyFileSource[0]   == "Z:\\FailPin_Device6.TXT" &&
              W7L4TesterTCP_CopyFileTarget[0]   == "D:\\HT9045_Log\\OSTestResult\\Device000123_06.TXT",
              "PART6: guard #1 false + guard #2 true -> only the FailPin copy runs, still to the same golden target name (golden :681 vs :692)");

        UninstallAllHooks();
    }

    // -----------------------------------------------------------------
    // PART 7 -- PRODUCTION PASS-THROUGH PROOF. Nothing above proves that the
    // seams do the REAL thing when no hook is installed, because everything
    // above installs hooks. This part proves it by experiment, on scratch
    // paths, through the three W7L4TesterTCP_Prove*Seam entry points -- which
    // call the very same TU-local seam functions the production body calls.
    //
    // This part deliberately does NOT call TesterTCP_CopyOSTestResult: that
    // function's paths are golden-hardcoded to D:\HT9045_Log\OSTestResult and
    // Z:\, and this suite must not create anything under D:\HT9045_Log.
    // -----------------------------------------------------------------
    std::printf("\n-- PART 7: with NO hook installed the seams really call the real functions --\n");
    {
        UninstallAllHooks();

        const char *pTemp = std::getenv("TEMP");
        AnsiString sBase  = (pTemp != NULL && pTemp[0] != '\0') ? AnsiString(pTemp) : AnsiString(".");
        AnsiString sRoot  = sBase + "\\" + AnsiString(W906_TestTmpName("w7l4_seamproof").c_str());   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h)
        AnsiString sDeep  = sRoot + "\\a\\b";                 // 2 levels -> needs a real ForceDirectories
        AnsiString sSrc   = sDeep + "\\seamproof_src.txt";
        AnsiString sDst   = sDeep + "\\seamproof_dst.txt";
        AnsiString sNever = sDeep + "\\seamproof_absent.txt";

        // Start clean (a previous interrupted run must not fake a pass).
        if (FileExists(sSrc))   DeleteFile(sSrc);
        if (FileExists(sDst))   DeleteFile(sDst);
        if (FileExists(sNever)) DeleteFile(sNever);
        if (DirectoryExists(sDeep)) RemoveDir(sDeep);
        if (DirectoryExists(sRoot + "\\a")) RemoveDir(sRoot + "\\a");
        if (DirectoryExists(sRoot)) RemoveDir(sRoot);

        W7L4TesterTCP_ResetSeamRecording();

        // (a) seam #1 with no hook -> the REAL MyForceDirectories runs.
        bool bAbsentBefore = !DirectoryExists(sDeep);
        W7L4TesterTCP_ProveForceDirectoriesSeam(sDeep, "W7L4-passthrough-proof");
        CHECK(bAbsentBefore && DirectoryExists(sDeep) &&
              W7L4TesterTCP_ForceDirectoriesCount == 1 &&
              W7L4TesterTCP_LastForceDirectoriesPath == sDeep,
              "PART7a: hook NULL -> seam #1 really created the (2-level) scratch directory, and still recorded the path");

        // (b) seam #2 with no hook -> the REAL CheckFileExist runs (true for a
        //     file that exists, false for one that does not).
        std::FILE *fp = std::fopen(sSrc.c_str(), "wb");
        if (fp != NULL) { std::fputs("w7l4 seam pass-through proof payload\n", fp); std::fclose(fp); }
        bool bRealTrue  = W7L4TesterTCP_ProveCheckFileExistSeam(sSrc);
        bool bRealFalse = W7L4TesterTCP_ProveCheckFileExistSeam(sNever);
        CHECK(bRealTrue == true && bRealFalse == false,
              "PART7b: hook NULL -> seam #2 really consulted the filesystem (true for an existing file, false for an absent one)");

        // (c) seam #3 with no hook -> the REAL CopyFile runs (bytes land).
        bool bDstAbsentBefore = !FileExists(sDst);
        bool bCopyOk          = W7L4TesterTCP_ProveCopyFileSeam(sSrc, sDst, false);
        bool bDstPresentAfter = FileExists(sDst);
        long lSrcLen = 0, lDstLen = 0;
        {
            std::FILE *f1 = std::fopen(sSrc.c_str(), "rb");
            if (f1 != NULL) { std::fseek(f1, 0, SEEK_END); lSrcLen = std::ftell(f1); std::fclose(f1); }
            std::FILE *f2 = std::fopen(sDst.c_str(), "rb");
            if (f2 != NULL) { std::fseek(f2, 0, SEEK_END); lDstLen = std::ftell(f2); std::fclose(f2); }
        }
        CHECK(bDstAbsentBefore && bCopyOk && bDstPresentAfter && lSrcLen > 0 && lSrcLen == lDstLen,
              "PART7c: hook NULL -> seam #3 really copied the file (destination appeared with the source's byte count)");

        // (d) and the hook, once installed, really does divert the same call --
        //     which is what makes (a)-(c) meaningful as a NULL-vs-hook contrast.
        W7L4TesterTCP_CheckFileExistHook = HookExist_AlwaysFalse;
        bool bDiverted = W7L4TesterTCP_ProveCheckFileExistSeam(sSrc);   // sSrc really exists
        W7L4TesterTCP_CheckFileExistHook = NULL;
        bool bRealAgain = W7L4TesterTCP_ProveCheckFileExistSeam(sSrc);
        CHECK(bDiverted == false && bRealAgain == true,
              "PART7d: the same probe of the same existing file answers false with a hook installed and true again once it is removed -- the hook is the only difference");

        // Teardown: leave nothing behind.
        if (FileExists(sSrc)) DeleteFile(sSrc);
        if (FileExists(sDst)) DeleteFile(sDst);
        RemoveDir(sDeep);
        RemoveDir(sRoot + "\\a");
        RemoveDir(sRoot);
        CHECK(!DirectoryExists(sRoot),
              "PART7e: scratch directory removed -- this suite leaves no artefacts anywhere");
    }

    UninstallAllHooks();
    W7L4TesterTCP_ResetSeamRecording();
    TestIF_File.iTestType = savedTestType;
    LastSet.iTester       = savedTester;

    // Self-check on this suite's own promise: it must not have created the
    // production folder. Reported as INFO (it is a property of the suite, not
    // of the translated function) but loud enough to notice.
    std::printf("\n%s  D:\\HT9045_Log\\OSTestResult %s after this run\n",
                DirectoryExists("D:\\HT9045_Log\\OSTestResult") ? "INFO-NOTE" : "INFO-OK",
                DirectoryExists("D:\\HT9045_Log\\OSTestResult")
                    ? "EXISTS (this suite never creates or touches it -- something else did)"
                    : "does not exist (this suite never creates it)");

    std::printf("\n=== %d PASS, %d FAIL (of %d) ===\n", g_pass, g_fail, g_pass + g_fail);
    return (g_fail == 0) ? 0 : 1;
}
